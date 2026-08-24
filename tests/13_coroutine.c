foo :: proc(coroutine: *Coroutine) #coroutine
{
	print 1;
	yield;

	print 2;
	yield;

	print 3;
}

main :: proc() -> int
{
	coroutine: Coroutine;

	foo(&coroutine);
	foo(&coroutine);
	foo(&coroutine);

	return 0;
}
