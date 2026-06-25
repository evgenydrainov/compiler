main :: proc() -> i64
{
	a: i64 = 10;
	b: i64 = 5;

	if a < b
		return 1;
	else if a > b
		return 0;

	return 2;
}
