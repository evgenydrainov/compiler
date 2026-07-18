// struct field access, stack allocation, pointer-to-struct, (*p).field.
main :: proc() -> i64
{
	// basic field set/read
	{
		p: Point;
		p.x = 3;
		p.y = 4;
		if p.x + p.y != 7 { return 1; }
	}

	// mutation through pointer parameter
	{
		p: Point;
		fill(&p);
		if p.x + p.y != 40 { return 2; }
	}

	// struct alongside scalar on stack (layout/offset sanity)
	{
		p: Point;
		a: i64 = 7;
		p.x = 1;
		p.y = 2;
		if a != 7 { return 3; }
		if p.x + p.y != 3 { return 4; }
	}

	// per-field independence
	{
		p: Point;
		p.x = 100;
		p.y = 200;
		p.x = p.x + 1;
		if p.x != 101 { return 5; }
		if p.y != 200 { return 6; }
	}

	// reading fields back through a pointer
	{
		p: Point;
		p.x = 11;
		p.y = 22;
		pp: *Point = &p;
		if (*pp).x != 11 { return 7; }
		if (*pp).y != 22 { return 8; }
		(*pp).x = 33;
		if p.x != 33 { return 9; }
	}

	return 0;
}

fill :: proc(p: *Point)
{
	(*p).x = 25;
	(*p).y = 15;
}

Point :: struct
{
	x: i64;
	y: i64;
}
