main :: proc()
{
	print factorial(5);
}

factorial :: proc(a: i64) -> i64
{
	if a <= 1
	{
		return a;
	}
	return a * factorial(a - 1);
}

