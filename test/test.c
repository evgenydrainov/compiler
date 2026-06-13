main :: proc()
{
	print factorial(5);
}

factorial :: proc(a: int)
{
	if a <= 1
	{
		return a;
	}
	return a * factorial(a - 1);
}