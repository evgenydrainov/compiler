// SHOULD FAIL: passing a []i64 where a []f32 is expected.
main :: proc() -> i64
{
	a: [4]i64;
	s := make_slice_i64(&a[0], a.count);

	takeFloats(s);
	return 0;
}

takeFloats :: proc(s: []f32)
{
}

make_slice_i64 :: proc(data: *i64, count: i64) -> []i64
{
	s: []i64;
	s.data  = data;
	s.count = count;
	return s;
}
