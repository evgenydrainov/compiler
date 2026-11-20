// SHOULD FAIL: a slice has exactly two fields, data and count.
main :: proc() -> i64
{
	a: [4]i64;
	s := make_slice_i64(&a[0], a.count);

	return s.length;
}

make_slice_i64 :: proc(data: *i64, count: i64) -> []i64
{
	s: []i64;
	s.data  = data;
	s.count = count;
	return s;
}
