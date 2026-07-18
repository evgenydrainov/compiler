// locals, assignment, shadowing, block scope, self-reference.
main :: proc() -> i64
{
	a: i64 = 5;
	b: i64 = a * a;
	if b != 25 { return 1; }

	// reassignment
	a = a + 1;
	if a != 6 { return 2; }

	// compound self-reference in initializer chain
	c: i64 = a + b;
	if c != 31 { return 3; }

	// uninitialized then assigned
	d: i64;
	d = 42;
	if d != 42 { return 4; }

	// block scope shadowing: inner does not leak
	x: i64 = 1;
	{
		x: i64 = 2;
		if x != 2 { return 5; }
	}
	if x != 1 { return 6; }

	// nested shadowing depth
	y: i64 = 10;
	{
		y: i64 = 20;
		{
			y: i64 = 30;
			if y != 30 { return 7; }
		}
		if y != 20 { return 8; }
	}
	if y != 10 { return 9; }

	// inner block mutates outer variable (no shadow)
	z: i64 = 100;
	{
		z = z + 1;
	}
	if z != 101 { return 10; }

	return 0;
}
