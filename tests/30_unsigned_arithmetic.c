main :: proc() -> i64
{
	a: u32 = 4000000000;
	b: u32 = 2;
	c: u32 = a / b;
	if c != 2000000000
	{
		return 1;
	}

	x: u32 = 0x80000000;
	y: u32 = x >> 1;
	if y != 0x40000000
	{
		return 2;
	}

	big: u32 = 0xFFFFFFFF;
	one: u32 = 1;
	if !(big > one)
	{
		return 3;
	}

	small: u8 = 200;
	small = small + 100;
	if cast(i64) small != 44
	{
		return 4;
	}

	return 0;
}
