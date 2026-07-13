main :: proc() -> i64
{
	a8: i8 = 3;
	b8: i8 = 5;
	if !(a8 < b8)
	{
		return 1;
	}

	a16: i16 = 100;
	b16: i16 = 50;
	if !(a16 > b16)
	{
		return 2;
	}

	a32: i32 = 7;
	b32: i32 = 7;
	if !(a32 <= b32)
	{
		return 3;
	}

	if !(a32 >= b32)
	{
		return 4;
	}

	a64: i64 = 10;
	b64: i64 = 20;
	if !(a64 < b64)
	{
		return 5;
	}

	return 0;
}
