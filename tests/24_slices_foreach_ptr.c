main :: proc() -> i64
{
	// ---- mutation through the pointer reaches the array ----
	{
		a: [4]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		s := make_slice_i64(&a[0], a.count);

		for *e in s
		{
			*e = *e * 10;
		}

		if a[0] != 10 { return 1; }
		if a[1] != 20 { return 2; }
		if a[2] != 30 { return 3; }
		if a[3] != 40 { return 4; }
	}

	// ---- reading through the pointer, in order ----
	{
		a: [4]i64;
		a[0] = 5;
		a[1] = 6;
		a[2] = 7;
		a[3] = 8;

		s := make_slice_i64(&a[0], a.count);

		seen  := 0;
		total := 0;
		order := 1;

		for *e in s
		{
			if *e != a[seen] { order = 0; }
			total += *e;
			seen  += 1;
		}

		if seen  != 4  { return 5; }
		if total != 26 { return 6; }
		if order != 1  { return 7; }
	}

	// ---- the pointers really are distinct, one per element ----
	{
		a: [3]i64;
		a[0] = 0;
		a[1] = 0;
		a[2] = 0;

		s := make_slice_i64(&a[0], a.count);

		first: *i64;
		last:  *i64;
		i := 0;

		for *e in s
		{
			if i == 0 { first = e; }
			last = e;
			i += 1;
		}

		*first = 11;
		*last  = 33;

		if a[0] != 11 { return 8; }
		if a[1] != 0  { return 9; }
		if a[2] != 33 { return 10; }
	}

	// ---- an empty slice still runs zero iterations ----
	{
		a: [4]i64;

		empty := make_slice_i64(&a[0], 0);

		n := 0;
		for *e in empty
		{
			n += 1;
			*e = 1;
		}
		if n != 0 { return 11; }
	}

	// ---- struct elements: the case this form exists for ----
	// this is world.c:55 rewritten as a foreach
	{
		w: World;
		w.num_bricks = 3;
		for i := 0; i < 12; i += 1
		{
			w.bricks[i].hp   = i + 1;
			w.bricks[i].dead = false;
		}

		live := make_slice_entity(&w.bricks[0], w.num_bricks);

		for *brick in live
		{
			brick.hp   = brick.hp * 2;
			brick.dead = true;
		}

		if w.bricks[0].hp   != 2     { return 12; }
		if w.bricks[2].hp   != 6     { return 13; }
		if w.bricks[0].dead != true  { return 14; }
		if w.bricks[2].dead != true  { return 15; }

		// only the sliced range was touched
		if w.bricks[3].hp   != 4     { return 16; }
		if w.bricks[3].dead != false { return 17; }
	}

	// ---- a pointer foreach over a slice parameter ----
	{
		a: [4]i64;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		s := make_slice_i64(&a[0], a.count);
		negate(s);

		if a[0] != -1 { return 18; }
		if a[3] != -4 { return 19; }
	}

	// ---- element sizes other than 8 ----
	{
		a: [4]i32;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;

		s := make_slice_i32(&a[0], a.count);

		for *e in s
		{
			*e = *e + 1;
		}

		if cast(i64) a[0] != 2 { return 20; }
		if cast(i64) a[3] != 5 { return 21; }
	}

	{
		// 3 bytes: a non-power-of-two stride
		a: [3]Three;
		for i := 0; i < 3; i += 1
		{
			a[i].a = cast(i8) i;
			a[i].b = 0;
			a[i].c = 0;
		}

		s := make_slice_three(&a[0], a.count);

		for *t in s
		{
			t.b = t.a;
			t.c = t.a;
		}

		if cast(i64) a[2].b != 2 { return 22; }
		if cast(i64) a[2].c != 2 { return 23; }
	}

	// ---- break and continue ----
	{
		a: [5]i64;
		for i := 0; i < 5; i += 1 { a[i] = i + 1; }

		s := make_slice_i64(&a[0], a.count);

		for *e in s
		{
			if *e == 4 { break; }
			*e = 0;
		}
		if a[0] != 0 { return 24; }
		if a[2] != 0 { return 25; }
		if a[3] != 4 { return 26; }
		if a[4] != 5 { return 27; }
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

negate :: proc(s: []i64)
{
	for *e in s
	{
		*e = -*e;
	}
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
	dead: bool;
}

World :: struct
{
	bricks: [12]Entity;
	num_bricks: i64;
}
