main :: proc() -> i64
{
	u: u8 = 200;

	s: i8 = cast(i8) u;
	if s != -56
	{
		return 1;
	}

	neg: i8 = -1;
	back: u8 = cast(u8) neg;
	if back != 255
	{
		return 2;
	}

	negBig: i32 = -1;
	byteVal: u8 = cast(u8) negBig;
	if byteVal != 255
	{
		return 3;
	}

	return 0;
}
