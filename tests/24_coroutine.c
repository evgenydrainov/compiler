#include "../modules/builtin.c"

foo :: proc(state: *CoroutineState) #coroutine
{
	print 1;
	yield;

	print 2;
	yield;

	print 3;
}

main :: proc() -> int
{
	state: CoroutineState;

	foo(&state);
	foo(&state);
	foo(&state);

	return 0;
}
