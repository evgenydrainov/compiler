main :: proc() -> i64
{
	if 10 - 5 - 2 != 3
	{
		return 1;
	}

	if 100 / 10 / 2 != 5
	{
		return 2;
	}

	if 20 - 5 - 3 - 2 != 10
	{
		return 3;
	}

	if 17 % 5 % 3 != 2
	{
		return 4;
	}

	return 0;
}
