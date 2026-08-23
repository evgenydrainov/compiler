// SHOULD FAIL: a dynamic array has data, count and capacity. nothing else.
main :: proc() -> i64
{
	a: [..]i64;
	array_add(&a, 1);

	return a.length;
}
