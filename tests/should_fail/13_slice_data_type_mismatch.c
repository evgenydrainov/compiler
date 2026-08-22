// SHOULD FAIL: .data on a []i64 is a *i64, not a pointer to anything else.
main :: proc() -> i64
{
	small: [4]i32;

	s: []i64;
	s.data  = &small[0];
	s.count = 4;

	return s[0];
}
