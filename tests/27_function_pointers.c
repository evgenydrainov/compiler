main :: proc() -> i64
{
	// ---- a procedure in a local, called and then replaced ----
	{
		f: proc(int, int) -> int;

		f = add;
		if f(3, 4) != 7 { return 1; }

		f = mul;
		if f(3, 4) != 12 { return 2; }
	}

	// ---- inferred from the initializer ----
	{
		f := add;
		if f(1, 1) != 2 { return 3; }
	}

	// ---- copied between variables; the name compares equal to the value ----
	{
		f := add;
		g := f;

		if g(2, 3) != 5 { return 4; }
		if f != g       { return 5; }
		if f != add     { return 6; }
	}

	// ---- passed as an argument ----
	// 'apply' calls through a parameter, which is the same lookup as a local
	{
		if apply(add, 5, 6) != 11 { return 7; }
		if apply(mul, 5, 6) != 30 { return 8; }
	}

	// ---- returned from a procedure ----
	{
		f := pick(true);
		if f(2, 3) != 5 { return 9; }

		g := pick(false);
		if g(2, 3) != 6 { return 10; }
	}

	// ---- the call target is itself a call ----
	{
		if pick(true)(4, 5) != 9 { return 11; }
	}

	// ---- a table, indexed and called in one expression ----
	{
		table: [2]proc(int, int) -> int;
		table[0] = add;
		table[1] = mul;

		total := 0;
		for i in 0..<2
		{
			total += table[i](3, 4);
		}
		if total != 19 { return 12; }
	}

	// ---- a global ----
	{
		g_handler = triple;
		if g_handler(5) != 15 { return 13; }
	}

	// ---- a struct field, whose signature names the struct it lives in ----
	{
		e: Entity;
		e.hp = 10;
		e.update = damage;

		e.update(&e);
		e.update(&e);
		if e.hp != 8 { return 14; }

		e.update = heal;

		e.update(&e);
		if e.hp != 9 { return 15; }
	}

	// ---- null, and comparison ----
	{
		f: proc(int, int) -> int = null;
		if f != null { return 16; }

		f = add;
		if f == null { return 17; }
		if f != add  { return 18; }

		f = mul;
		if f == add { return 19; }
	}

	// ---- an aggregate return, through the hidden pointer ----
	{
		f := make_v2;

		v := f(3, 4);
		if v.x != 3 { return 20; }
		if v.y != 4 { return 21; }
	}

	// ---- an aggregate parameter, passed by reference to a copy ----
	{
		v: V2;
		v.x = 3;
		v.y = 4;

		f := sum_v2;
		if f(v) != 7 { return 22; }
	}

	// ---- no parameters, no return value ----
	{
		ticks = 0;

		f: proc() = tick;
		f();
		f();

		if ticks != 2 { return 23; }
	}

	// ---- floats, written here and #foreign ----
	{
		f := fabsf;
		if f(-2.5) != 2.5 { return 24; }

		g := sqrtf;
		if g(9.0) != 3.0 { return 25; }

		h := fminf;
		if h(3.0, 5.0) != 3.0 { return 26; }
	}

	// ---- a dynamic array of them ----
	{
		a: [..]proc(int, int) -> int;
		defer array_free(&a);

		array_add(&a, add);
		array_add(&a, mul);
		array_add(&a, add);

		total := 0;
		for f in a
		{
			total += f(2, 3);
		}
		if total != 16 { return 27; }
	}

	// ---- the arguments are still evaluated exactly once ----
	{
		calls = 0;

		f := add;
		if f(bump(3), bump(4)) != 7 { return 29; }
		if calls != 2               { return 30; }
	}

	return 0;
}

// ---- targets ------------------------------------------------------------

add :: proc(a: int, b: int) -> int
{
	return a + b;
}

mul :: proc(a: int, b: int) -> int
{
	return a * b;
}

triple :: proc(x: int) -> int
{
	return x * 3;
}

sum_v2 :: proc(v: V2) -> int
{
	return v.x + v.y;
}

make_v2 :: proc(a: int, b: int) -> V2
{
	v: V2;
	v.x = a;
	v.y = b;
	return v;
}

damage :: proc(e: *Entity)
{
	e.hp -= 1;
}

heal :: proc(e: *Entity)
{
	e.hp += 1;
}

tick :: proc()
{
	ticks += 1;
}

bump :: proc(x: int) -> int
{
	calls += 1;
	return x;
}

// ---- procedures that take and return procedures -------------------------

apply :: proc(f: proc(int, int) -> int, a: int, b: int) -> int
{
	return f(a, b);
}

// the return type's arrow binds to the inner 'proc': this returns a procedure
pick :: proc(first: bool) -> proc(int, int) -> int
{
	if first
	{
		return add;
	}

	return mul;
}

// ---- state --------------------------------------------------------------

g_handler: proc(int) -> int;

ticks: int;
steps: int;
calls: int;

V2 :: struct
{
	x: int;
	y: int;
}

Entity :: struct
{
	hp:     int;
	update: proc(*Entity);
}
