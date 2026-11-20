// SHOULD FAIL: there is no implicit [..]T -> []T conversion.
// the slice has to be built explicitly: make_slice_i64(a.data, a.count)
sum :: proc(s: []i64) -> i64
{
	total := 0;
	for x in s
	{
		total += x;
	}
	return total;
}

main :: proc() -> i64
{
	a: [..]i64;
	array_add(&a, 1);

	return sum(a);
}
