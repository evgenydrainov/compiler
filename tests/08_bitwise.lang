// &, |, ^, ~, <<, >>, precedence and combinations.
main :: proc() -> i64
{
	if (12 & 10) != 8   { return 1; }
	if (12 | 10) != 14  { return 2; }
	if (12 ^ 10) != 6   { return 3; }
	if (~0) != -1       { return 4; }
	if (~5) != -6       { return 5; }

	if (1 << 4) != 16   { return 6; }
	if (256 >> 4) != 16 { return 7; }
	if (1 << 0) != 1    { return 8; }
	if (0 << 5) != 0    { return 9; }

	// shift accumulations
	if (1 << 10) != 1024 { return 10; }
	if (1024 >> 10) != 1 { return 11; }

	// precedence: shift vs or, and vs xor
	if (1 << 2 | 1) != 5 { return 12; }
	if (12 & 10 | 1) != 9 { return 13; }
	if (0xF0 & 0x0F) != 0 { return 14; }
	if (0xF0 | 0x0F) != 0xFF { return 15; }

	// identities
	if (255 ^ 255) != 0 { return 16; }
	if (255 ^ 0) != 255 { return 17; }
	if (170 & 170) != 170 { return 18; }

	// combine with arithmetic
	if ((1 << 3) + (1 << 1)) != 10 { return 19; }

	return 0;
}
