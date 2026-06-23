main :: proc() -> i64
{
	{
		p: Point;
		p.x = 3;
		p.y = 4;

		if p.x + p.y != 7
		{
			return 1;
		}
	}

	{
		p: Point;
		foo(&p);

		if p.x + p.y != 40
		{
			return 2;
		}
	}

	return 0;
}

foo :: proc(p: *Point)
{
	(*p).x = 25;
	(*p).y = 15;
}

Point :: struct
{
	x: i64;
	y: i64;
}
