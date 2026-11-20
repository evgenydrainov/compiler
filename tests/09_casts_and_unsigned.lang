// explicit casts (narrow/widen, sign, float <-> int), unsigned arithmetic
// and wraparound.
main :: proc() -> i64
{
	// narrowing truncates
	big: i64 = 300;
	small: i8 = cast(i8) big;
	if small != 44 { return 1; }

	// sign extension on widen
	neg: i8 = -1;
	widened: i64 = cast(i64) neg;
	if widened != -1 { return 2; }

	// widening a value that fits is lossless
	x: i32 = 1000;
	if cast(i16) x != 1000 { return 3; }

	// unsigned -> signed reinterpretation via cast
	u: u8 = 200;
	s: i8 = cast(i8) u;
	if s != -56 { return 4; }

	// signed -> unsigned
	negb: i8 = -1;
	back: u8 = cast(u8) negb;
	if back != 255 { return 5; }

	// narrow negative i32 to u8
	negBig: i32 = -1;
	byteVal: u8 = cast(u8) negBig;
	if byteVal != 255 { return 6; }

	// unsigned division uses unsigned semantics
	a: u32 = 4000000000;
	b: u32 = 2;
	if a / b != 2000000000 { return 7; }

	// logical shift on unsigned (no sign fill)
	hi: u32 = 0x80000000;
	if (hi >> 1) != 0x40000000 { return 8; }

	// unsigned comparison: 0xFFFFFFFF is large, not -1
	bigu: u32 = 0xFFFFFFFF;
	one: u32 = 1;
	if !(bigu > one) { return 9; }

	// unsigned 8-bit wraparound
	w: u8 = 200;
	w = w + 100;
	if cast(i64) w != 44 { return 10; }

	// round-trip: i64 -> i8 -> i64 keeps low byte, sign-extended
	v: i64 = 0x1234567890ABCDEF;
	lo: i8 = cast(i8) v;      // 0xEF -> -17
	if cast(i64) lo != -17 { return 11; }

	// narrowing in expression position: the cast itself must truncate,
	// there is no store to a narrow slot to do it for us
	wide: i64 = 300;
	if cast(i8) wide != 44 { return 12; }

	wide = 200;
	if cast(i8) wide != -56 { return 13; }   // 0xC8, sign-extended

	wide = 456;
	if cast(u8) wide != 200 { return 14; }   // 0x1C8 -> 0xC8, zero-extended

	// narrowing a negative value in expression position
	negb2: i32 = -1;
	if cast(u8) negb2 != 255 { return 15; }
	if cast(i16) negb2 != -1 { return 16; }

	// truncation to the wider integer sizes
	huge: i64 = 0x100000007;
	if cast(i32) huge != 7 { return 17; }
	if cast(u32) huge != 5 + 2 { return 18; }

	huge = 0x12345678;
	if cast(i16) huge != 0x5678 { return 19; }

	// widening in expression position
	small2: i8 = -5;
	if cast(i32) small2 != -5 { return 20; }

	// the cast result feeds arithmetic directly
	wide = 300;
	if cast(i8) wide * 2 != 88 { return 21; }

	// a cast as a call argument crosses the ABI already truncated
	wide = 300;
	if identity8(cast(i8) wide) != 44 { return 22; }

	// narrow then widen without an intervening store
	wide = 300;
	if cast(i64) (cast(i8) wide) != 44 { return 23; }

	// ---- float <-> integer, every width ----
	// all constants below are exactly representable in binary

	d: f64 = 300.75f64;
	if cast(i64) d != 300 { return 24; }
	if cast(i32) d != 300 { return 25; }
	if cast(i16) d != 300 { return 26; }
	if cast(i8)  d != 44  { return 27; }   // 300 truncates to 0x2C
	if cast(u8)  d != 44  { return 28; }
	if cast(u32) d != 300 { return 29; }

	// float -> int truncates toward zero, not down
	nd: f64 = -2.75f64;
	if cast(i32) nd != -2 { return 30; }
	if cast(i64) nd != -2 { return 31; }

	f: f32 = 130.5;
	if cast(i64) f != 130  { return 32; }
	if cast(i16) f != 130  { return 33; }
	if cast(u8)  f != 130  { return 34; }
	if cast(i8)  f != -126 { return 35; }  // 130 -> 0x82, sign-extended

	// narrow integers widen before converting
	i8v: i8 = -5;
	if cast(f64) i8v != -5.0f64 { return 36; }
	if cast(f32) i8v != -5.0    { return 37; }

	u8v: u8 = 200;
	if cast(f64) u8v != 200.0f64 { return 38; }
	if cast(f32) u8v != 200.0    { return 39; }

	// unsigned sources are zero-extended, not treated as negative
	u32v: u32 = 4000000000;
	if cast(f64) u32v != 4000000000.0f64 { return 40; }

	i64v: i64 = 16777216;
	if cast(f32) i64v != 16777216.0 { return 41; }

	// identity casts and the f32 <-> f64 round trip
	if cast(f64) d != 300.75f64         { return 42; }
	if cast(f32) f != 130.5             { return 43; }
	if cast(f64) f != 130.5f64          { return 44; }
	if cast(f32) (cast(f64) f) != 130.5 { return 45; }

	return 0;
}

identity8 :: proc(v: i8) -> i64
{
	return cast(i64) v;
}
