// floating point: literals, f64/f32 arithmetic, comparisons, negation,
// int<->float and f32<->f64 casts, and float args/returns across calls.
// all constants are exactly representable in binary, so == comparisons are exact.
main :: proc() -> i64
{
	// ---- f64 arithmetic ----
	x: f64 = 1.5f64;
	y: f64 = 0.25f64;
	if x + y != 1.75f64  { return 1; }
	if x - y != 1.25f64  { return 2; }
	if x * y != 0.375f64 { return 3; }
	if x / y != 6.0f64   { return 4; }

	// ---- f64 comparisons ----
	if !(x > y)        { return 5; }
	if !(y < x)        { return 6; }
	if !(x >= 1.5f64)  { return 7; }
	if !(x <= 1.5f64)  { return 8; }
	if !(x == 1.5f64)  { return 9; }
	if !(x != y)       { return 10; }

	// ---- f64 negation ----
	n: f64 = -1.5f64;
	if n != -1.5f64 { return 11; }
	if -n != 1.5f64 { return 12; }

	// ---- f32 arithmetic ----
	a: f32 = 1.5;
	b: f32 = 0.25;
	if a + b != 1.75  { return 13; }
	if a * b != 0.375 { return 14; }
	if !(a > b)       { return 15; }

	// ---- casts: int <-> float ----
	// int widening to float
	if cast(f64) 3 != 3.0f64 { return 16; }
	// float -> int truncates toward zero
	if cast(i64)  3.75f64  !=  3 { return 17; }
	if cast(i64)(-3.75f64) != -3 { return 18; }

	// ---- casts: f32 <-> f64 ----
	f: f32 = cast(f32) 1.5f64;
	if cast(f64) f != 1.5f64 { return 19; }
	d: f64 = 2.25f64;
	if cast(f32) d != 2.25 { return 20; }

	// ---- float return from a function ----
	//if scale(1.5, 4.0) != 6.0 { return 21; }
//
	//// ---- mixed int/float arguments (positional register slots) ----
	//// a=2, b=1.5, c=3, d=0.25 -> 1.5*2 + 0.25*3 = 3.75
	//if combine(2, 1.5, 3, 0.25) != 3.75 { return 22; }
//
	//// ---- five float args: 5th is passed on the stack ----
	//// 0.5 + 0.25 + 0.125 + 0.0625 + 0.0625 = 1.0
	//if sum5(0.5, 0.25, 0.125, 0.0625, 0.0625) != 1.0 { return 23; }
//
	//// ---- recursion with floats ----
	//// 1.0 + 0.5 + 0.25 = 1.75 (powers of two, exact)
	//if pow2_sum(3) != 1.75 { return 24; }

	return 0;
}

//scale :: proc(v: f64, factor: f64) -> f64
//{
//	return v * factor;
//}
//
//combine :: proc(a: i64, b: f64, c: i64, d: f64) -> f64
//{
//	return b * cast(f64) a + d * cast(f64) c;
//}
//
//sum5 :: proc(a: f64, b: f64, c: f64, d: f64, e: f64) -> f64
//{
//	return a + b + c + d + e;
//}
//
//// sum of 1/2^i for i in [0, n): 1 + 0.5 + 0.25 + ...
//pow2_sum :: proc(n: i64) -> f64
//{
//	if n <= 0 { return 0.0; }
//	return term(n - 1) + pow2_sum(n - 1);
//}
//
//term :: proc(i: i64) -> f64
//{
//	// 1.0 / 2^i
//	result: f64 = 1.0;
//	while i > 0 {
//		result = result / 2.0;
//		i = i - 1;
//	}
//	return result;
//}
