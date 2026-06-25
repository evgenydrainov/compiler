main :: proc() -> i64
{
	a: i64 = 10;
	b: i64 = 5;
	c: i64 = 1;

	if a > b && b > c
	{
		return 0;
	}

	return 1;
}
