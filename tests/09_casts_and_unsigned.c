// explicit casts (narrow/widen, sign), unsigned arithmetic and wraparound.
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

	return 0;
}
