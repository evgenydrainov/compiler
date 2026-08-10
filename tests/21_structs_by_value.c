main :: proc() -> i64
{
	// ---- register-sized struct: 8 bytes, fits in a register ----
	{
		s := makeSmall(3, 4);
		if s.a != 3 { return 1; }
		if s.b != 4 { return 2; }
		if sumSmall(s) != 7 { return 3; }
	}

	// ---- 16-byte struct returned into a fresh variable ----
	{
		v := makeV2(10, 20);
		if v.x != 10 { return 4; }
		if v.y != 20 { return 5; }
	}

	// ---- returned into a variable that already exists ----
	{
		v: V2;
		v.x = 1;
		v.y = 2;
		v = makeV2(30, 40);
		if v.x != 30 { return 6; }
		if v.y != 40 { return 7; }
	}

	// ---- returned straight into a struct field and an array element ----
	{
		n: Nested;
		n.min = makeV2(1, 2);
		n.max = makeV2(5, 9);
		if n.min.x != 1 { return 8; }
		if n.max.y != 9 { return 9; }

		arr: [2]V2;
		arr[0] = makeV2(7, 8);
		arr[1] = makeV2(11, 12);
		if arr[0].x != 7  { return 10; }
		if arr[1].y != 12 { return 11; }
	}

	// ---- 24-byte struct ----
	{
		v := makeV3(1, 2, 3);
		if v.x + v.y + v.z != 6 { return 12; }
		if sumV3(v) != 6 { return 13; }
	}

	// ---- sizes that are not a multiple of 8: the copy needs a tail ----
	{
		t := makeThree(1, 2, 3);
		if sumThree(t) != 6 { return 14; }

		w := makeTwelve(100, 200, 300);
		if sumTwelve(w) != 600 { return 15; }
	}

	// ---- a struct whose layout depends on field padding ----
	{
		m := makeMixed(7, 1234567890123);
		if cast(i64) m.a != 7 { return 16; }
		if m.b != 1234567890123 { return 17; }
	}

	// ---- by value means the callee gets a copy ----
	// this is the check that separates a real by-value parameter from silently
	// handing the callee a pointer to the caller's variable
	{
		v := makeV2(1, 2);
		clobber(v);
		if v.x != 1 { return 18; }
		if v.y != 2 { return 19; }

		s := makeSmall(3, 4);
		clobberSmall(s);
		if s.a != 3 { return 20; }
		if s.b != 4 { return 21; }
	}

	// ---- two results from the same procedure are independent copies ----
	{
		a := makeV2(1, 2);
		b := makeV2(5, 6);
		a.x = 100;
		if b.x != 5 { return 22; }
	}

	// ---- a call result feeding straight into another call ----
	{
		if sumV2(makeV2(4, 5)) != 9 { return 23; }
		if sumV2(passThrough(makeV2(6, 7))) != 13 { return 24; }
	}

	// ---- a field read directly off a call result ----
	{
		if makeV2(30, 40).x != 30 { return 25; }
		if makeV2(30, 40).y != 40 { return 26; }
		if makeNested(1, 2, 3, 4).max.y != 4 { return 27; }
	}

	// ---- struct arguments alongside enough scalars to spill onto the stack ----
	{
		if mix5(1, makeV2(2, 3), 4, 5, 6) != 21 { return 28; }
		if twoStructs(makeV2(1, 2), makeV2(3, 4)) != 10 { return 29; }
	}

	// ---- a struct return from a procedure with enough parameters that the
	// hidden return pointer pushes real arguments onto the stack ----
	{
		v := build5(1, 2, 3, 4, 5);
		if v.x != 1 { return 30; }
		if v.y != 9 { return 31; }   // 2 + 3 + 4
		if v.z != 5 { return 32; }
	}

	// ---- the returned value is snapshotted before the defers run ----
	{
		v := makeThenClobber();
		if v.x != 5 { return 33; }
		if v.y != 6 { return 34; }
	}

	// ---- a struct containing an array, by value ----
	{
		bag := makeBag(10, 20, 30);
		if sumBag(bag) != 60 { return 35; }

		clobberBag(bag);
		if sumBag(bag) != 60 { return 36; }
	}

	// ---- a struct containing structs, by value ----
	{
		n := makeNested(1, 2, 4, 6);
		if area(n) != 12 { return 37; }
	}

	// ---- recursion that returns structs ----
	{
		v := countdown(4);
		if v.x != 4  { return 38; }
		if v.y != 10 { return 39; }   // 4 + 3 + 2 + 1
	}

	return 0;
}

makeSmall :: proc(a: i32, b: i32) -> Small
{
	s: Small;
	s.a = a;
	s.b = b;
	return s;
}

sumSmall :: proc(s: Small) -> i64
{
	return cast(i64) s.a + cast(i64) s.b;
}

// 8 bytes: the ABI passes and returns this one in a register
Small :: struct
{
	a: i32;
	b: i32;
}

clobberSmall :: proc(s: Small)
{
	s.a = 99;
	s.b = 99;
}

makeV2 :: proc(x: i64, y: i64) -> V2
{
	v: V2;
	v.x = x;
	v.y = y;
	return v;
}

sumV2 :: proc(v: V2) -> i64
{
	return v.x + v.y;
}

passThrough :: proc(v: V2) -> V2
{
	return v;
}

clobber :: proc(v: V2)
{
	v.x = 999;
	v.y = 999;
}

V2 :: struct
{
	x: i64;
	y: i64;
}

makeNested :: proc(minX: i64, minY: i64,
				   maxX: i64, maxY: i64) -> Nested
{
	n: Nested;
	n.min = makeV2(minX, minY);
	n.max = makeV2(maxX, maxY);
	return n;
}

Nested :: struct
{
	min: V2;
	max: V2;
}

makeV3 :: proc(x: i64, y: i64, z: i64) -> V3
{
	v: V3;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

sumV3 :: proc(v: V3) -> i64
{
	return v.x + v.y + v.z;
}

V3 :: struct
{
	x: i64;
	y: i64;
	z: i64;
}

makeThree :: proc(a: i8, b: i8, c: i8) -> Three
{
	t: Three;
	t.a = a;
	t.b = b;
	t.c = c;
	return t;
}

sumThree :: proc(t: Three) -> i64
{
	return cast(i64) t.a + cast(i64) t.b + cast(i64) t.c;
}

// 3 bytes: neither register-sized nor a multiple of 8
Three :: struct
{
	a: i8;
	b: i8;
	c: i8;
}

makeTwelve :: proc(a: i32, b: i32, c: i32) -> Twelve
{
	w: Twelve;
	w.a = a;
	w.b = b;
	w.c = c;
	return w;
}

sumTwelve :: proc(w: Twelve) -> i64
{
	return cast(i64) w.a + cast(i64) w.b + cast(i64) w.c;
}

// 12 bytes: a copy loop that only moves 8 bytes at a time needs a tail
Twelve :: struct
{
	a: i32;
	b: i32;
	c: i32;
}

makeMixed :: proc(a: i8, b: i64) -> Mixed
{
	m: Mixed;
	m.a = a;
	m.b = b;
	return m;
}

// the offset of 'b' depends on whether fields are padded to their alignment
Mixed :: struct
{
	a: i8;
	b: i64;
}

area :: proc(n: Nested) -> i64
{
	return (n.max.x - n.min.x) * (n.max.y - n.min.y);
}

makeBag :: proc(a: i64, b: i64, c: i64) -> Bag
{
	bag: Bag;
	bag.items[0] = a;
	bag.items[1] = b;
	bag.items[2] = c;
	bag.count = 3;
	return bag;
}

sumBag :: proc(bag: Bag) -> i64
{
	total := 0;
	for i := 0; i < bag.count; i += 1
	{
		total += bag.items[i];
	}
	return total;
}

clobberBag :: proc(bag: Bag)
{
	bag.items[0] = 0;
	bag.items[1] = 0;
	bag.items[2] = 0;
	bag.count = 0;
}

mix5 :: proc(a: i64, v: V2, b: i64, c: i64, d: i64) -> i64
{
	return a + v.x + v.y + b + c + d;
}

twoStructs :: proc(a: V2, b: V2) -> i64
{
	return a.x + a.y + b.x + b.y;
}

build5 :: proc(a: i64, b: i64, c: i64, d: i64, e: i64) -> V3
{
	v: V3;
	v.x = a;
	v.y = b + c + d;
	v.z = e;
	return v;
}

// the defers must not be able to touch the value that was already returned
makeThenClobber :: proc() -> V2
{
	v: V2;
	v.x = 5;
	v.y = 6;

	defer v.x = 999;
	defer v.y = 999;

	return v;
}

countdown :: proc(n: i64) -> V2
{
	v: V2;

	if n <= 0
	{
		return v;
	}

	rest := countdown(n - 1);

	v.x = n;
	v.y = n + rest.y;

	return v;
}

Bag :: struct
{
	items: [3]i64;
	count: i64;
}
