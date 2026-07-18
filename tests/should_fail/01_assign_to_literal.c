// SHOULD FAIL: assignment to a non-lvalue.
main :: proc() -> i64
{
	5 = 3;
	return 0;
}
