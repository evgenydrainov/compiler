// SHOULD FAIL: a slice is a view. it does not own its memory and has no
// capacity, so it cannot grow.
main :: proc() -> i64
{
	/*backing: [4]i64;

	s: []i64;
	s.data  = &backing[0];
	s.count = 4;

	array_add(&s, 5);*/

	// TODO: do typechecking on array_add() somehow

	error();

	return s.count;
}
