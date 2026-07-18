// integer arithmetic, precedence, associativity, division/modulo edges.
main :: proc() -> i64
{
	// precedence
	if 2 + 3 * 4 != 14 { return 1; }
	if (2 + 3) * 4 != 20 { return 2; }
	if 2 * 3 + 4 * 5 != 26 { return 3; }

	// subtraction / left-associativity
	if 10 - 3 != 7 { return 4; }
	if 20 - 5 - 3 - 2 != 10 { return 5; }
	if 100 / 10 / 2 != 5 { return 6; }

	// division truncates toward zero
	if 100 / 7 != 14 { return 7; }
	if 7 / 2 != 3 { return 8; }
	if -7 / 2 != -3 { return 9; }
	if 7 / -2 != -3 { return 10; }
	if -7 / -2 != 3 { return 11; }

	// modulo, including sign of result (follows dividend)
	if 56 % 10 != 6 { return 12; }
	if -56 % 10 != -6 { return 13; }
	if 56 % -10 != 6 { return 14; }
	if 17 % 5 % 3 != 2 { return 15; }

	// unary minus and mixed
	if -(3 + 4) != -7 { return 16; }
	if -5 + 10 != 5 { return 17; }
	if 3 * -4 != -12 { return 18; }

	// deep nesting / redundant parens
	if ((((1 + 2)) * ((3 + 4))) != 21) { return 19; }

	// identity / zero
	if 0 * 999 != 0 { return 20; }
	if 999 - 999 != 0 { return 21; }
	if 0 / 5 != 0 { return 22; }
	if 0 % 5 != 0 { return 23; }

	return 0;
}
