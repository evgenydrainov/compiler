main :: proc() -> i64
{
	// ---- a fresh dynamic array is empty and owns nothing ----
	{
		a: [..]i64;

		if a.count    != 0    { return 1; }
		if a.capacity != 0    { return 2; }
		if a.data     != null { return 3; }
	}

	// ---- a single append ----
	{
		a: [..]i64;

		array_add(&a, 42);

		if a.count    != 1    { return 4; }
		if a.capacity != 8    { return 5; }
		if a[0]       != 42   { return 6; }
		if a.data     == null { return 7; }

		array_free(&a);
	}

	// ---- a hundred appends, crossing four growth steps ----
	{
		a: [..]i64;
		defer array_free(&a);

		for i in 0..<100
		{
			array_add(&a, i * 3);
		}

		if a.count    != 100 { return 8; }
		if a.capacity != 128 { return 9; }

		ok := 1;
		for i in 0..<100
		{
			if a[i] != i * 3 { ok = 0; }
		}
		if ok != 1 { return 10; }
	}

	// ---- growth preserves what was already there ----
	// element 0 is written before any reallocation and read after several
	{
		a: [..]i64;
		defer array_free(&a);

		array_add(&a, 111);

		for i in 0..<64
		{
			array_add(&a, i);
		}

		if a.count != 65  { return 11; }
		if a[0]    != 111 { return 12; }
		if a[64]   != 63  { return 13; }
	}

	// ---- clearing keeps the buffer ----
	{
		a: [..]i64;
		defer array_free(&a);

		for i in 0..<10
		{
			array_add(&a, i);
		}

		cap := a.capacity;
		buf := a.data;

		a.count = 0;

		if a.count    != 0   { return 14; }
		if a.capacity != cap { return 15; }
		if a.data     != buf { return 16; }

		array_add(&a, 7);
		if a.count != 1 { return 17; }
		if a[0]    != 7 { return 18; }
	}

	// ---- reserving up front means the appends never reallocate ----
	{
		a: [..]i64;
		defer array_free(&a);

		array_reserve(&a, 100);

		if a.count    != 0   { return 19; }
		if a.capacity != 128 { return 20; }

		buf := a.data;

		for i in 0..<100
		{
			array_add(&a, i);
		}

		if a.count != 100 { return 21; }
		if a.data  != buf { return 22; }
	}

	// ---- writing an element, and the address of one ----
	{
		a: [..]i64;
		defer array_free(&a);

		for i in 0..<5
		{
			array_add(&a, 0);
		}

		a[2] = 99;
		if a[2] != 99 { return 23; }

		p := &a[3];
		*p = 77;
		if a[3] != 77 { return 24; }

		if p != &a[3] { return 25; }
		if p == &a[2] { return 26; }
	}

	// ---- element sizes other than 8 ----
	{
		a: [..]i32;
		defer array_free(&a);

		for i in 0..<10
		{
			array_add(&a, cast(i32) i);
		}

		if a.count != 10 { return 27; }
		if cast(i64) a[9] != 9 { return 28; }

		total := 0;
		for i in 0..<10
		{
			total += cast(i64) a[i];
		}
		if total != 45 { return 29; }
	}

	{
		// 1 byte, and enough of them to reallocate six times
		a: [..]u8;
		defer array_free(&a);

		for i in 0..<300
		{
			array_add(&a, cast(u8) (i % 7));
		}

		if a.count != 300 { return 30; }
		if cast(i64) a[299] != 5 { return 31; }   // 299 % 7
		if cast(i64) a[0]   != 0 { return 32; }
	}

	{
		// 3 bytes: a non-power-of-two stride
		a: [..]Three;
		defer array_free(&a);

		for i in 0..<20
		{
			array_add(&a, makeThree(cast(i8) i, 0, cast(i8)(i + 1)));
		}

		if a.count != 20 { return 33; }
		if cast(i64) a[19].a != 19 { return 34; }
		if cast(i64) a[19].c != 20 { return 35; }
		if cast(i64) a[0].a  != 0  { return 36; }
	}

	{
		// 24 bytes: the same size as the header itself
		a: [..]Big;
		defer array_free(&a);

		for i in 0..<20
		{
			array_add(&a, makeBig(i, i * 2, i * 3));
		}

		if a.count != 20 { return 37; }
		if a[19].a != 19 { return 38; }
		if a[19].c != 57 { return 39; }
		if a[0].b  != 0  { return 40; }
	}

	// ---- foreach, by value and by pointer ----
	{
		a: [..]i64;
		defer array_free(&a);

		for i in 0..<10
		{
			array_add(&a, i + 1);
		}

		total := 0;
		for x in a
		{
			total += x;
		}
		if total != 55 { return 41; }

		for *x in a
		{
			*x = *x * 2;
		}
		if a[0] != 2  { return 42; }
		if a[9] != 20 { return 43; }

		// binding by value does not write back
		for x in a
		{
			x = 0;
		}
		if a[0] != 2 { return 44; }
	}

	// ---- an empty array runs zero iterations and never touches .data ----
	{
		a: [..]i64;

		n := 0;
		for x in a
		{
			n += 1;
			n += x;
		}
		if n != 0 { return 45; }
	}

	// ---- appending during a foreach ----
	// the loop iterates a copy of the header, so it sees the count as it was
	// when the loop started. the reserve is what keeps this well-defined: it
	// stops the buffer from moving under the copy.
	{
		a: [..]i64;
		defer array_free(&a);

		array_reserve(&a, 64);

		for i in 0..<4
		{
			array_add(&a, i);
		}

		n := 0;
		for x in a
		{
			n += 1;
			array_add(&a, 100);
		}

		if n         != 4 { return 46; }
		if a.count   != 8 { return 47; }
		if a[7]      != 100 { return 48; }
	}

	// ---- passing by value: the callee gets a copy of the header ----
	{
		a: [..]i64;
		defer array_free(&a);

		for i in 0..<5
		{
			array_add(&a, i + 1);
		}

		if sumOf(a) != 15 { return 49; }

		truncate(a);
		if a.count != 5 { return 50; }
	}

	// ---- passing by pointer: the callee can grow it ----
	{
		a: [..]i64;
		defer array_free(&a);

		fill(&a, 10);

		if a.count != 10 { return 51; }
		if a[9]    != 9  { return 52; }
	}

	// ---- returning one by value ----
	{
		a := makeRange(6);
		defer array_free(&a);

		if a.count != 6 { return 53; }
		if a[0]    != 0 { return 54; }
		if a[5]    != 5 { return 55; }
	}

	// ---- as a struct field ----
	// this is the base form that broke slice indexing the first time round
	{
		b: Bag;
		defer array_free(&b.items);

		for i in 0..<5
		{
			array_add(&b.items, i * 10);
		}

		if b.items.count != 5  { return 56; }
		if b.items[3]    != 30 { return 57; }
		if b.tag         != 0  { return 58; }

		b.items[1] = 7;
		if b.items[1] != 7 { return 59; }
	}

	// ---- a dynamic array of dynamic arrays ----
	{
		rows: [..][..]i64;
		defer array_free(&rows);

		for i in 0..<3
		{
			r: [..]i64;
			for j in 0..<4
			{
				array_add(&r, i * 10 + j);
			}
			array_add(&rows, r);
		}

		if rows.count    != 3  { return 60; }
		if rows[2].count != 4  { return 61; }
		if rows[2][3]    != 23 { return 62; }
		if rows[0][0]    != 0  { return 63; }

		// freeing the outer array does not free the inner ones
		for i in 0..<3
		{
			array_free(&rows[i]);
		}
	}

	// ---- the slice view ----
	{
		a: [..]i64;
		defer array_free(&a);

		for i in 0..<8
		{
			array_add(&a, i + 1);
		}

		s := make_slice_i64(a.data, a.count);

		if s.count != 8 { return 64; }
		if s[0]    != 1 { return 65; }
		if s[7]    != 8 { return 66; }

		total := 0;
		for x in s
		{
			total += x;
		}
		if total != 36 { return 67; }

		// the slice is a view, not a copy
		s[2] = 99;
		if a[2] != 99 { return 68; }
	}

	// ---- array_free resets to the empty state, and it is reusable ----
	{
		a: [..]i64;

		for i in 0..<10
		{
			array_add(&a, i);
		}

		array_free(&a);

		if a.count    != 0    { return 69; }
		if a.capacity != 0    { return 70; }
		if a.data     != null { return 71; }

		array_add(&a, 5);
		if a.count != 1 { return 72; }
		if a[0]    != 5 { return 73; }

		array_free(&a);
	}

	// ---- defer array_free runs at the end of its block ----
	{
		a: [..]i64;
		{
			defer array_free(&a);

			for i in 0..<10
			{
				array_add(&a, i);
			}
			if a.count != 10 { return 74; }
		}
		if a.count != 0 { return 75; }
	}

	return 0;
}

// ---- helpers ------------------------------------------------------------

sumOf :: proc(a: [..]i64) -> i64
{
	total := 0;
	for x in a
	{
		total += x;
	}
	return total;
}

// writing count in the callee must not reach the caller
truncate :: proc(a: [..]i64)
{
	a.count = 1;
}

fill :: proc(a: *[..]i64, n: i64)
{
	for i in 0..<n
	{
		array_add(a, i);
	}
}

makeRange :: proc(n: i64) -> [..]i64
{
	a: [..]i64;
	for i in 0..<n
	{
		array_add(&a, i);
	}
	return a;
}

make_slice_i64 :: proc(data: *i64, count: i64) -> []i64
{
	s: []i64;
	s.data  = data;
	s.count = count;
	return s;
}

makeThree :: proc(a: i8, b: i8, c: i8) -> Three
{
	t: Three;
	t.a = a;
	t.b = b;
	t.c = c;
	return t;
}

makeBig :: proc(a: i64, b: i64, c: i64) -> Big
{
	v: Big;
	v.a = a;
	v.b = b;
	v.c = c;
	return v;
}

Three :: struct
{
	a: i8;
	b: i8;
	c: i8;
}

Big :: struct
{
	a: i64;
	b: i64;
	c: i64;
}

Bag :: struct
{
	items: [..]i64;
	tag: i64;
}
