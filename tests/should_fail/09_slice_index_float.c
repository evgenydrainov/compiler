// SHOULD FAIL: indexing a slice with a float.
main :: proc() -> i64
{
	a: [4]i64;
	s := make_slice_i64(&a[0], a.count);

	i := 1.0;
	return s[i];
}

make_slice_i64 :: proc(data: *i64, count: i64) -> []i64
{
	s: []i64;
	s.data  = data;
	s.count = count;
	return s;
}
