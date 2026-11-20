// break / continue in loops.
main :: proc() -> int
{
	// break exits early
	sum := 0;
	for i := 0; i < 100; i += 1
	{
		if i == 5
		{
			break;
		}
		sum += i;
	}
	if sum != 10
	{
		return 1;  // 0+1+2+3+4
	}

	// continue skips iteration body remainder
	evensum := 0;
	for i := 0; i < 10; i += 1
	{
		if i % 2 != 0
		{
			continue;
		}
		evensum += i;
	}
	if evensum != 20
	{
		return 2;  // 0+2+4+6+8
	}

	// break in while
	w := 0;
	while 1 == 1
	{
		w += 1;
		if w == 3 { break; }
	}
	if w != 3 { return 3; }

	return 0;
}
