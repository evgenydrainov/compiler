// SHOULD FAIL: calling a proc with the wrong number of arguments.
main :: proc() -> i64
{
	return add(1);
}

add :: proc(a: i64, b: i64) -> i64 { return a + b; }
