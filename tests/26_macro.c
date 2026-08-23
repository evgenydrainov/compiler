main :: proc() -> i64
{
	// ---- the simplest expansion ----
	{
		x := 0;

		set_to(&x, 42);
		if x != 42 { return 1; }

		set_to(&x, x + 1);
		if x != 43 { return 2; }
	}

	// ---- mutating a struct through a pointer argument ----
	{
		p: Pair;
		p.a = 1;
		p.b = 2;

		scale(&p, 3);

		if p.a != 3 { return 3; }
		if p.b != 6 { return 4; }
	}

	// ---- an argument used many times is still evaluated once ----
	// 'v' appears four times in quad's body
	{
		calls = 0;

		r := 0;
		quad(&r, bump(5));

		if r     != 20 { return 5; }
		if calls != 1  { return 6; }
	}

	// ---- arguments are evaluated before the body ----
	{
		order = 0;

		r := 0;
		seq(&r, mark(1));

		if order != 12 { return 7; }
		if r     != 3  { return 8; }
	}

	// ---- a macro-local name does not touch the caller's ----
	{
		tmp := 5;

		r := 0;
		stash(&r);

		if r   != 99 { return 9; }
		if tmp != 5  { return 10; }
	}

	// ---- two expansions of the same macro in one block ----
	{
		r1 := 0;
		r2 := 0;

		stash(&r1);
		stash(&r2);

		if r1 != 99 { return 11; }
		if r2 != 99 { return 12; }
	}

	// ---- an expansion nested inside another expansion ----
	// both stash2 and stash declare 'tmp'
	{
		r1 := 0;
		r2 := 0;

		stash2(&r1, &r2);

		if r1 != 106 { return 13; }   // 99 + 7
		if r2 != 99  { return 14; }
	}

	// ---- a macro expanding a macro expanding a macro ----
	{
		x := 0;

		inc(&x);
		if x != 1 { return 15; }

		inc_twice(&x);
		if x != 3 { return 16; }

		inc_four(&x);
		if x != 7 { return 17; }
	}

	// ---- one macro, three argument types ----
	{
		a := 1;
		b := 2;
		swap(&a, &b);
		if a != 2 { return 18; }
		if b != 1 { return 19; }
	}

	{
		c: u8;
		d: u8;
		c = 7;
		d = 9;

		swap(&c, &d);

		if cast(i64) c != 9 { return 20; }
		if cast(i64) d != 7 { return 21; }
	}

	{
		// an aggregate: the macro-local copy is a struct copy
		p: Pair;
		p.a = 1;
		p.b = 2;

		q: Pair;
		q.a = 3;
		q.b = 4;

		swap(&p, &q);

		if p.a != 3 { return 22; }
		if p.b != 4 { return 23; }
		if q.a != 1 { return 24; }
		if q.b != 2 { return 25; }
	}

	// ---- a macro with no arguments ----
	{
		calls = 42;
		reset_calls();
		if calls != 0 { return 26; }
	}

	// ---- expansion inside control flow ----
	{
		x := 0;

		if true { inc(&x); }
		if x != 1 { return 27; }

		i := 0;
		while i < 5
		{
			inc(&x);
			i = i + 1;
		}
		if x != 6 { return 28; }

		for j in 0..<3
		{
			inc(&x);
		}
		if x != 9 { return 29; }

		{
			{
				inc(&x);
			}
		}
		if x != 10 { return 30; }
	}

	// ---- expansion inside a foreach body ----
	{
		a: [..]int;
		defer array_free(&a);

		array_add(&a, 1);
		array_add(&a, 2);
		array_add(&a, 3);

		total := 0;
		for v in a
		{
			add_to(&total, v);
		}
		if total != 6 { return 31; }

		for *v in a
		{
			scale_int(v, 10);
		}
		if a[0] != 10 { return 32; }
		if a[2] != 30 { return 33; }
	}

	// ---- an argument that is itself a complex expression ----
	{
		a: [..]int;
		defer array_free(&a);

		array_add(&a, 10);
		array_add(&a, 20);

		calls = 0;

		r := 0;
		quad(&r, a[bump(1)]);

		if r     != 80 { return 34; }
		if calls != 1  { return 35; }
	}

	// ---- sizeof, which array_reserve's expansion needs ----
	{
		x := 0;
		if sizeof(x) != 8 { return 36; }

		c: u8;
		if sizeof(c) != 1 { return 37; }

		p: Pair;
		if sizeof(p) != 16 { return 38; }

		// the operand is not evaluated: a.data is null here
		a: [..]Pair;
		if sizeof(*a.data) != 16 { return 39; }
		if a.data != null  { return 40; }
	}

	// ---- array_add and array_reserve, now expanded from builtin.c ----
	{
		a: [..]int;
		defer array_free(&a);

		array_reserve(&a, 100);
		if a.capacity != 128 { return 41; }

		buf := a.data;

		for i in 0..<50
		{
			array_add(&a, i);
		}

		if a.count != 50 { return 42; }
		if a[49]   != 49 { return 43; }
		if a.data  != buf { return 44; }
	}

	{
		// a struct element type: the size comes from sizeof, not a literal
		b: [..]Pair;
		defer array_free(&b);

		for i in 0..<10
		{
			array_add(&b, makePair(i, i * 2));
		}

		if b.count != 10 { return 45; }
		if b[9].a  != 9  { return 46; }
		if b[9].b  != 18 { return 47; }
		if b[0].a  != 0  { return 48; }
	}

	{
		// array_add's own argument is evaluated once too
		a: [..]int;
		defer array_free(&a);

		calls = 0;
		array_add(&a, bump(7));

		if a.count != 1 { return 49; }
		if a[0]    != 7 { return 50; }
		if calls   != 1 { return 51; }
	}

	return 0;
}

// ---- macros ------------------------------------------------------------

set_to :: macro(p, v)
{
	*p = v;
}

scale :: macro(p, k)
{
	p.a = p.a * k;
	p.b = p.b * k;
}

scale_int :: macro(p, k)
{
	*p = *p * k;
}

add_to :: macro(p, v)
{
	*p = *p + v;
}

quad :: macro(dst, v)
{
	*dst = v + v + v + v;
}

seq :: macro(dst, x)
{
	*dst = x + mark(2);
}

stash :: macro(dst)
{
	tmp := 99;
	*dst = tmp;
}

stash2 :: macro(d1, d2)
{
	tmp := 7;
	stash(d1);
	stash(d2);
	*d1 = *d1 + tmp;
}

inc :: macro(p)
{
	*p = *p + 1;
}

inc_twice :: macro(p)
{
	inc(p);
	inc(p);
}

inc_four :: macro(p)
{
	inc_twice(p);
	inc_twice(p);
}

swap :: macro(pa, pb)
{
	t := *pa;
	*pa = *pb;
	*pb = t;
}

reset_calls :: macro()
{
	calls = 0;
}

// ---- helpers -----------------------------------------------------------

calls: int;
order: int;

bump :: proc(x: int) -> int
{
	calls = calls + 1;
	return x;
}

mark :: proc(v: int) -> int
{
	order = order * 10 + v;
	return v;
}

makePair :: proc(a: int, b: int) -> Pair
{
	p: Pair;
	p.a = a;
	p.b = b;
	return p;
}

Pair :: struct
{
	a: int;
	b: int;
}
