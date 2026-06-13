foo :: proc()
{
	i: int = 0;
	while i < 5
	{
		print i;
		i = i + 1;
	}
}

main :: proc()
{
	print 100;
	foo();

	print 200;
	foo();

	print 300;
	foo();
}
