main :: proc() -> i64
{
	big: i64 = 300;

	small: i8 = cast(i8) big;
	if small != 44
	{
		return 1;
	}

	neg: i8 = -1;
	widened: i64 = cast(i64) neg;
	if widened != -1
	{
		return 2;
	}

	x: i32 = 1000;
	if cast(i16) x != 1000
	{
		return 3;
	}

	return 0;
}
