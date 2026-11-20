// SHOULD FAIL: array_add grows the array, so it needs a pointer to it.
// passing the array by value would append to a copy that is thrown away.
main :: proc() -> i64
{
	a: [..]i64;

	array_add(a, 1);

	return a.count;
}
