// comparisons across int sizes, &&, ||, !, chained boolean logic.
main :: proc() -> i64
{
	// comparison operators, i64
	if !(1 < 2)   { return 1; }
	if !(2 > 1)   { return 2; }
	if !(2 <= 2)  { return 3; }
	if !(2 >= 2)  { return 4; }
	if !(1 == 1)  { return 5; }
	if !(1 != 2)  { return 6; }
	if 2 < 1      { return 7; }
	if 1 > 2      { return 8; }

	// all integer widths compare correctly
	a8: i8 = 3;    b8: i8 = 5;    if !(a8 < b8)  { return 9; }
	a16: i16 = 100; b16: i16 = 50; if !(a16 > b16) { return 10; }
	a32: i32 = 7;  b32: i32 = 7;  if !(a32 <= b32) { return 11; }
	if !(a32 >= b32) { return 12; }
	a64: i64 = 10; b64: i64 = 20; if !(a64 < b64) { return 13; }

	// logical AND
	if !(10 > 5 && 5 > 1) { return 14; }
	if (10 > 5 && 5 > 100) { return 15; }
	if (1 > 10 && 1 > 0)   { return 16; }

	// logical OR
	if !(10 > 5 || 5 > 100) { return 17; }
	if !(1 > 10 || 5 > 1)   { return 18; }
	if (1 > 10 || 2 > 20)   { return 19; }

	// NOT
	if !(!(1 > 2)) { return 20; }
	if !(0 == 0)   { return 21; }

	// combined precedence: && binds tighter than ||
	if !(0 == 1 && 1 == 1 || 1 == 1) { return 22; }
	if !(1 == 1 || 1 == 1 && 0 == 1) { return 23; }

	// comparison result usable as branch across nesting
	x: i64 = 5;
	if x > 0 && x < 10 && x != 3 {
		if x != 5 { return 24; }
	} else {
		return 25;
	}

	return 0;
}
