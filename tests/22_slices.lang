main :: proc() -> i64
{
	// ---- the compile-time count of a fixed array ----
	{
		a: [5]i64;
		if a.count != 5 { return 1; }

		b: [3]V2;
		if b.count != 3 { return 2; }

		// the outer length of a multi-dimensional array, and the inner one
		g: [3][4]i64;
		if g.count    != 3 { return 3; }
		if g[0].count != 4 { return 4; }

		// an array that is a field of a struct
		w: World;
		if w.bricks.count != 12 { return 5; }

		// it is a constant, so it survives arithmetic
		if a.count * 2 + 1 != 11 { return 6; }
	}

	// ---- construction ----
	{
		a: [5]i64;
		a[0] = 10;
		a[1] = 20;
		a[2] = 30;
		a[3] = 40;
		a[4] = 50;

		s: []i64 = make_slice_i64(&a[0], a.count);
		if s.count != 5 { return 7; }

		// .data is a *T, and indexing a *T already works
		if s.data[0] != 10 { return 8; }
		if s.data[4] != 50 { return 9; }

		// every element reads back through the slice
		for i := 0; i < 5; i += 1
		{
			if s[i] != a[i] { return 10; }
		}

		// a slice type is inferable
		t := s;
		if t.count != 5 { return 11; }
	}

	// ---- a slice is a view, not a copy ----
	{
		a: [3]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;

		s: []i64 = make_slice_i64(&a[0], a.count);

		s[1] = 99;
		if a[1] != 99 { return 12; }

		a[2] = 77;
		if s[2] != 77 { return 13; }
	}

	// ---- the fields are mutable ----
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i * 10; }

		s: []i64 = make_slice_i64(&a[0], a.count);

		// truncating by writing count
		s.count = 3;
		if s.count != 3  { return 14; }
		if s[2]    != 20 { return 15; }

		// re-pointing by writing data
		s.data  = &a[2];
		s.count = 3;
		if s[0] != 20 { return 16; }
		if s[2] != 40 { return 17; }

		// building one from a declaration with no initialiser
		t: []i64;
		t.data  = &a[1];
		t.count = 2;
		if t.count != 2  { return 18; }
		if t[0]    != 10 { return 19; }
		if t[1]    != 20 { return 20; }

		// a hand-built slice is a view like any other
		t[0] = 111;
		if a[1] != 111 { return 21; }

		// fields copied from another slice
		u: []i64;
		u.data  = t.data;
		u.count = t.count;
		if u[0] != 111 { return 22; }

		// zero count, and it is not an error
		u.count = 0;
		if u.count != 0 { return 23; }
	}

	// ---- make_slice ----
	{
		a: [5]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;
		a[4] = 5;

		whole := make_slice_i64(&a[0], a.count);
		if whole.count != 5 { return 24; }
		if whole[0]    != 1 { return 25; }
		if whole[4]    != 5 { return 26; }

		// a sub-slice from the middle
		sub := make_slice_i64(&a[1], 3);
		if sub.count != 3 { return 27; }
		if sub[0]    != 2 { return 28; }
		if sub[2]    != 4 { return 29; }

		// still a view
		sub[0] = 222;
		if a[1] != 222 { return 30; }

		// an empty one
		empty := make_slice_i64(&a[0], 0);
		if empty.count != 0 { return 31; }

		// taking the address of a slice element, not an array element
		s := make_slice_i64(&a[0], a.count);
		mid := make_slice_i64(&s[2], 2);
		if mid.count != 2 { return 32; }
		if mid[0]    != 3 { return 33; }
		if mid[1]    != 4 { return 34; }

		// and from a variable index
		i := 3;
		late := make_slice_i64(&s[i], 2);
		if late.count != 2 { return 35; }
		if late[0]    != 4 { return 36; }
	}

	// ---- a slice over memory from malloc ----
	{
		p := malloc_i64(32);   // 4 * sizeof(i64)
		p[0] = 1;
		p[1] = 2;
		p[2] = 3;
		p[3] = 4;

		heap := make_slice_i64(p, 4);
		if heap.count != 4 { return 37; }
		if heap[0]    != 1 { return 38; }
		if heap[3]    != 4 { return 39; }
		if sliceSum(heap) != 10 { return 40; }

		free_i64(p);
	}

	// ---- passing a slice to a procedure ----
	{
		a: [4]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		s := make_slice_i64(&a[0], a.count);

		if sliceCount(s) != 4  { return 41; }
		if sliceSum(s)   != 10 { return 42; }

		// mutation through a slice parameter reaches the array in the caller
		addOne(s);
		if a[0] != 2 { return 43; }
		if a[3] != 5 { return 44; }

		// writing count inside the callee must not disturb the caller: the
		// parameter is a copy of the pair, even though it views the same array
		truncate(s);
		if s.count != 4 { return 45; }

		// a slice argument alongside enough scalars to spill onto the stack
		if mix6(1, 2, 3, 4, s, 5) != 29 { return 46; }

		// a call result passed straight as an argument
		if sliceSum(make_slice_i64(&a[2], 2)) != 9 { return 47; }
	}

	// ---- returning a slice ----
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i * 10; }

		s := make_slice_i64(&a[0], a.count);

		f := firstN(s, 3);
		if f.count != 3  { return 48; }
		if f[0]    != 0  { return 49; }
		if f[2]    != 20 { return 50; }

		// takes a slice and returns one derived from it
		l := lastN(s, 2);
		if l.count != 2  { return 51; }
		if l[0]    != 30 { return 52; }
		if l[1]    != 40 { return 53; }

		// a returned slice still points at the array it came from
		f[1] = 111;
		if a[1] != 111 { return 54; }

		// a call result fed straight into another call
		if sliceSum(lastN(s, 2)) != 70 { return 55; }
	}

	// ---- element sizes other than 8 ----
	{
		// 4 bytes: the lea scaling path
		a: [4]i32;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		s := make_slice_i32(&a[0], a.count);
		if s.count != 4 { return 56; }
		if cast(i64) s[3] != 4 { return 57; }

		s[1] = 20;
		if cast(i64) a[1] != 20 { return 58; }

		m := make_slice_i32(&a[1], 2);
		if m.count != 2 { return 59; }
		if cast(i64) m[0] != 20 { return 60; }
		if cast(i64) m[1] != 3  { return 61; }
	}

	{
		// 1 byte
		a: [3]u8;
		a[0] = 7;
		a[1] = 8;
		a[2] = 9;

		s := make_slice_u8(&a[0], a.count);
		if s.count != 3 { return 62; }
		if cast(i64) s[2] != 9 { return 63; }

		t := make_slice_u8(&a[1], 2);
		if cast(i64) t[0] != 8 { return 64; }
		if cast(i64) t[1] != 9 { return 65; }
	}

	{
		// 16 bytes: struct elements, and a stride the lea path still handles
		a: [3]V2;
		a[0] = makeV2(1, 2);
		a[1] = makeV2(3, 4);
		a[2] = makeV2(5, 6);

		s := make_slice_v2(&a[0], a.count);
		if s.count != 3 { return 66; }
		if s[1].x != 3 { return 67; }
		if s[2].y != 6 { return 68; }

		// a struct element read out by value
		v := s[0];
		if v.x + v.y != 3 { return 69; }

		// written back through the slice
		s[0] = makeV2(70, 80);
		if a[0].x != 70 { return 70; }

		t := make_slice_v2(&a[1], 2);
		if t.count != 2 { return 71; }
		if t[0].x != 3 { return 72; }
		if t[1].y != 6 { return 73; }
	}

	{
		// 3 bytes: a non-power-of-two stride, so scaling needs imul
		a: [4]Three;
		a[0] = makeThree(1, 2, 3);
		a[1] = makeThree(4, 5, 6);
		a[2] = makeThree(7, 8, 9);
		a[3] = makeThree(10, 11, 12);

		s := make_slice_three(&a[0], a.count);
		if s.count != 4 { return 74; }
		if cast(i64) s[0].a != 1  { return 75; }
		if cast(i64) s[3].c != 12 { return 76; }

		t := make_slice_three(&a[2], 2);
		if cast(i64) t[0].a != 7  { return 77; }
		if cast(i64) t[1].b != 11 { return 78; }
	}

	// ---- a slice of slices ----
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i * 10; }

		rows: [2][]i64;
		rows[0] = make_slice_i64(&a[0], 2);
		rows[1] = make_slice_i64(&a[2], 3);

		// the element type is []i64, so &rows[0] is a *[]i64
		rr: [][]i64;
		rr.data  = &rows[0];
		rr.count = rows.count;

		if rr.count    != 2  { return 79; }
		if rr[0].count != 2  { return 80; }
		if rr[1].count != 3  { return 81; }
		if rr[0][1]    != 10 { return 82; }
		if rr[1][2]    != 40 { return 83; }

		// still a view all the way down
		rr[1][0] = 222;
		if a[2] != 222 { return 84; }

		// the fields of an element of a slice of slices are writable too
		rr[1].count = 1;
		if rr[1].count   != 1 { return 85; }
		if rows[1].count != 1 { return 86; }
	}

	// ---- a slice as a struct field ----
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i * 3; }

		sp: Span;
		sp.tag   = 7;
		sp.items = make_slice_i64(&a[1], 3);

		if sp.tag         != 7 { return 87; }
		if sp.items.count != 3 { return 88; }
		if sp.items[0]    != 3 { return 89; }
		if sp.items[2]    != 9 { return 90; }

		// through the field, into the array
		sp.items[1] = 60;
		if a[2] != 60 { return 91; }

		// the fields of a slice field are writable
		sp.items.count = 2;
		if sp.items.count != 2 { return 92; }
		sp.items.count = 3;

		// the whole struct by value: the slice inside it is copied shallowly,
		// so the copy still views the same array
		if spanSum(sp) != 72 { return 93; }   // 3 + 60 + 9
	}

	// ---- the motivating case: a view into an array that lives in a struct ----
	{
		w: World;
		w.num_bricks = 3;
		for i := 0; i < 12; i += 1 { w.bricks[i].hp = i + 1; }

		live := make_slice_entity(&w.bricks[0], w.num_bricks);
		if live.count != 3 { return 94; }
		if live[0].hp != 1 { return 95; }
		if live[2].hp != 3 { return 96; }

		// a view into the field, not a copy of it
		live[0].hp = 50;
		if w.bricks[0].hp != 50 { return 97; }

		if hpSum(live) != 55 { return 98; }

		// the whole array, using its compile-time count
		all := make_slice_entity(&w.bricks[0], w.bricks.count);
		if all.count != 12 { return 99; }
		if hpSum(all) != 127 { return 100; }   // 50 + 2..12
	}

	// ---- the returned slice is snapshotted before the defers run ----
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i; }

		s := make_slice_i64(&a[0], a.count);

		d := headThenClobber(s);
		if d.count != 2 { return 101; }
		if d[0]    != 0 { return 102; }
		if d[1]    != 1 { return 103; }
	}

	// ---- assignment between slices is a shallow copy of the pair ----
	{
		a: [4]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		x := make_slice_i64(&a[0], a.count);
		y := make_slice_i64(&a[2], 2);

		x = y;
		if x.count != 2 { return 104; }
		if x[0]    != 3 { return 105; }

		// x and y are two independent pairs that happen to view the same
		// elements: writing the fields of one must not disturb the other
		y.count = 1;
		y.data  = &a[0];
		if x.count != 2 { return 106; }
		if x[0]    != 3 { return 107; }
	}

	return 0;
}

// ---- make_slice, one per element type until there are generics ----------

make_slice_i64 :: proc(data: *i64, count: i64) -> []i64
{
	s: []i64;
	s.data  = data;
	s.count = count;
	return s;
}

make_slice_i32 :: proc(data: *i32, count: i64) -> []i32
{
	s: []i32;
	s.data  = data;
	s.count = count;
	return s;
}

make_slice_u8 :: proc(data: *u8, count: i64) -> []u8
{
	s: []u8;
	s.data  = data;
	s.count = count;
	return s;
}

make_slice_v2 :: proc(data: *V2, count: i64) -> []V2
{
	s: []V2;
	s.data  = data;
	s.count = count;
	return s;
}

make_slice_three :: proc(data: *Three, count: i64) -> []Three
{
	s: []Three;
	s.data  = data;
	s.count = count;
	return s;
}

make_slice_entity :: proc(data: *Entity, count: i64) -> []Entity
{
	s: []Entity;
	s.data  = data;
	s.count = count;
	return s;
}

// ---- helpers ------------------------------------------------------------

malloc_i64 :: proc(size: i64) -> *i64 #foreign "malloc";
free_i64   :: proc(p: *i64)           #foreign "free";

sliceCount :: proc(s: []i64) -> i64
{
	return s.count;
}

sliceSum :: proc(s: []i64) -> i64
{
	total := 0;
	for i := 0; i < s.count; i += 1
	{
		total += s[i];
	}
	return total;
}

addOne :: proc(s: []i64)
{
	for i := 0; i < s.count; i += 1
	{
		s[i] += 1;
	}
}

// the parameter is a copy of the pair, so this is invisible to the caller
truncate :: proc(s: []i64)
{
	s.count = 1;
}

// s is the 5th argument, so it arrives on the stack
mix6 :: proc(a: i64, b: i64, c: i64, d: i64, s: []i64, e: i64) -> i64
{
	return a + b + c + d + sliceSum(s) + e;
}

firstN :: proc(s: []i64, n: i64) -> []i64
{
	return make_slice_i64(s.data, n);
}

lastN :: proc(s: []i64, n: i64) -> []i64
{
	return make_slice_i64(&s[s.count - n], n);
}

headThenClobber :: proc(s: []i64) -> []i64
{
	r := make_slice_i64(s.data, 2);

	defer r.count = 99;
	defer r.data  = &s[3];

	return r;
}

spanSum :: proc(sp: Span) -> i64
{
	return sliceSum(sp.items);
}

hpSum :: proc(s: []Entity) -> i64
{
	total := 0;
	for i := 0; i < s.count; i += 1
	{
		total += s[i].hp;
	}
	return total;
}

makeV2 :: proc(x: i64, y: i64) -> V2
{
	v: V2;
	v.x = x;
	v.y = y;
	return v;
}

makeThree :: proc(a: i8, b: i8, c: i8) -> Three
{
	t: Three;
	t.a = a;
	t.b = b;
	t.c = c;
	return t;
}

Span :: struct
{
	items: []i64;
	tag: i64;
}

V2 :: struct
{
	x: i64;
	y: i64;
}

// 3 bytes: the stride is not a power of two
Three :: struct
{
	a: i8;
	b: i8;
	c: i8;
}

Entity :: struct
{
	hp: i64;
}

World :: struct
{
	bricks: [12]Entity;
	num_bricks: i64;
}
