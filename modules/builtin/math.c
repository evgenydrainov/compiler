max_i64 :: proc(a: i64, b: i64) -> i64
{
	if a < b return b;
	return a;
}

min_i64 :: proc(a: i64, b: i64) -> i64
{
	if a < b return a;
	return b;
}

clamp_i64 :: proc(x: i64, low: i64, high: i64) -> i64
{
	if x < low return low;
	if x > high return high;
	return x;
}

sign_i64 :: proc(x: i64) -> i64
{
	if x > 0 return 1;
	if x < 0 return -1;
	return 0;
}



max_f32 :: proc(a: f32, b: f32) -> f32
{
	if a < b return b;
	return a;
}

min_f32 :: proc(a: f32, b: f32) -> f32
{
	if a < b return a;
	return b;
}

clamp_f32 :: proc(x: f32, low: f32, high: f32) -> f32
{
	if x < low return low;
	if high < x return high;
	return x;
}

sign_f32 :: proc(x: f32) -> f32
{
	if x > 0 return 1;
	if x < 0 return -1;
	return 0;
}
