main :: proc()
{
	print 100;
	a: int = foo();

	print 200;
	b: int = foo();

	print 300;
	c: int = foo();

	return a + b + c;
}

foo :: proc()
{
	i: int = 0;
	while i < 5
	{
		print i;
		i = i + 1;
	}

	return i;
}
