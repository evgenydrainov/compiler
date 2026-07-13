main :: proc() -> i64
{
	if (12 & 10) != 8
	{
		return 1;
	}

	if (12 | 10) != 14
	{
		return 2;
	}

	if (12 ^ 10) != 6
	{
		return 3;
	}

	if (~0) != -1
	{
		return 4;
	}

	if (1 << 4) != 16
	{
		return 5;
	}

	if (256 >> 4) != 16
	{
		return 6;
	}

	if (1 << 2 | 1) != 5
	{
		return 7;
	}

	return 0;
}
