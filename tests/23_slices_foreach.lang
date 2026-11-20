main :: proc() -> i64
{
	// ---- visits every element, in order ----
	{
		a: [5]i64;
		a[0] = 2;
		a[1] = 4;
		a[2] = 6;
		a[3] = 8;
		a[4] = 10;

		s := make_slice_i64(&a[0], a.count);

		seen  := 0;
		total := 0;
		order := 1;
		for e in s
		{
			if e != a[seen] { order = 0; }
			total += e;
			seen  += 1;
		}
		if seen  != 5  { return 1; }
		if total != 30 { return 2; }
		if order != 1  { return 3; }
	}

	// ---- an empty slice runs zero iterations ----
	{
		a: [4]i64;
		a[0] = 1;

		empty := make_slice_i64(&a[0], 0);

		n := 0;
		for e in empty
		{
			n += 1;
			n += e;
		}
		if n != 0 { return 4; }
	}

	// ---- a single element ----
	{
		a: [4]i64;
		a[0] = 7;

		one := make_slice_i64(&a[0], 1);

		n     := 0;
		total := 0;
		for e in one
		{
			n     += 1;
			total += e;
		}
		if n     != 1 { return 5; }
		if total != 7 { return 6; }
	}

	// ---- binding by value does not mutate the array ----
	{
		a: [3]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;

		s := make_slice_i64(&a[0], a.count);

		for e in s
		{
			e = 999;
		}

		if a[0] != 1 { return 7; }
		if a[1] != 2 { return 8; }
		if a[2] != 3 { return 9; }
	}

	// ---- over a call result, evaluated exactly once ----
	// a per-iteration re-evaluation would call tail() more than once
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i + 1; }

		s := make_slice_i64(&a[0], a.count);

		calls = 0;
		total := 0;
		for e in tail(s)
		{
			total += e;
		}
		if calls != 1  { return 10; }
		if total != 14 { return 11; }   // 2 + 3 + 4 + 5
	}

	// ---- the loop reads the count once, so truncating inside is ignored ----
	// this pins down that $it is a copy of the pair, not an alias of s
	{
		a: [4]i64;
		for i := 0; i < 4; i += 1 { a[i] = i + 1; }

		s := make_slice_i64(&a[0], a.count);

		n := 0;
		for e in s
		{
			s.count = 1;
			n += 1;
		}
		if n != 4 { return 12; }
	}

	// ---- a slice parameter ----
	{
		a: [4]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		s := make_slice_i64(&a[0], a.count);
		if sumByForeach(s) != 10 { return 13; }
	}

	// ---- element sizes other than 8 ----
	{
		a: [4]i32;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		s := make_slice_i32(&a[0], a.count);

		total := 0;
		for e in s
		{
			total += cast(i64) e;
		}
		if total != 10 { return 14; }
	}

	{
		a: [3]u8;
		a[0] = 7;
		a[1] = 8;
		a[2] = 9;

		s := make_slice_u8(&a[0], a.count);

		total := 0;
		for e in s
		{
			total += cast(i64) e;
		}
		if total != 24 { return 15; }
	}

	// ---- struct elements: the element is a copy, so writing it is inert ----
	{
		a: [3]Entity;
		for i := 0; i < 3; i += 1 { a[i].hp = i + 1; }

		s := make_slice_entity(&a[0], a.count);

		total := 0;
		for e in s
		{
			total += e.hp;
			e.hp = 999;
		}
		if total != 6 { return 16; }
		if a[0].hp != 1 { return 17; }
		if a[2].hp != 3 { return 18; }
	}

	{
		// 3 bytes: a non-power-of-two stride
		a: [4]Three;
		a[0] = makeThree(1, 2, 3);
		a[1] = makeThree(4, 5, 6);
		a[2] = makeThree(7, 8, 9);
		a[3] = makeThree(10, 11, 12);

		s := make_slice_three(&a[0], a.count);

		total := 0;
		for t in s
		{
			total += cast(i64) t.a;
		}
		if total != 22 { return 19; }   // 1 + 4 + 7 + 10
	}

	// ---- nested foreach over two slices ----
	{
		a: [3]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;

		b: [2]i64;
		b[0] = 10;
		b[1] = 20;

		sa := make_slice_i64(&a[0], a.count);
		sb := make_slice_i64(&b[0], b.count);

		total := 0;
		for x in sa
		{
			for y in sb
			{
				total += x * y;
			}
		}
		if total != 180 { return 20; }   // (1+2+3) * (10+20)
	}

	// ---- break and continue still work inside a slice foreach ----
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i + 1; }

		s := make_slice_i64(&a[0], a.count);

		total := 0;
		for e in s
		{
			if e == 4 { break; }
			total += e;
		}
		if total != 6 { return 21; }   // 1 + 2 + 3

		total = 0;
		for e in s
		{
			if e == 3 { continue; }
			total += e;
		}
		if total != 12 { return 22; }   // 1 + 2 + 4 + 5
	}

	// ---- the range form is unaffected ----
	{
		total := 0;
		for i in 0..<5
		{
			total += i;
		}
		if total != 10 { return 23; }
	}

	return 0;
}

// ---- helpers ------------------------------------------------------------

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

calls: int;

tail :: proc(s: []i64) -> []i64
{
	calls += 1;
	return make_slice_i64(&s[1], s.count - 1);
}

sumByForeach :: proc(s: []i64) -> i64
{
	total := 0;
	for e in s
	{
		total += e;
	}
	return total;
}

makeThree :: proc(a: i8, b: i8, c: i8) -> Three
{
	t: Three;
	t.a = a;
	t.b = b;
	t.c = c;
	return t;
}

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
