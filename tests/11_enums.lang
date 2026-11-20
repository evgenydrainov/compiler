// enum values, equality, assignment, use in branches and comparisons,
// and implicit enumerator syntax (.Red) in every context with an expected enum type.

Color :: enum
{
	Red;
	Green;
	Blue;
};

// TODO: maybe resolve global vars in a separate pass, after structs and enums
global_color: Color;

main :: proc() -> i64
{
	color: Color;

	color = Color.Red;
	if color != Color.Red { return 1; }
	if color == Color.Green { return 2; }

	color = Color.Green;
	if color != Color.Green { return 3; }

	color = Color.Blue;
	if color != Color.Blue { return 4; }

	// enum drives control flow
	color = Color.Green;
	code: i64 = 0;
	if color == Color.Red {
		code = 1;
	} else if color == Color.Green {
		code = 2;
	} else {
		code = 3;
	}
	if code != 2 { return 5; }

	// distinctness of all members
	if Color.Red == Color.Green { return 6; }
	if Color.Green == Color.Blue { return 7; }
	if Color.Red == Color.Blue { return 8; }

	// ---- implicit enum syntax ----

	// variable declaration with an explicit type
	{
		c: Color = .Blue;
		if c != Color.Blue { return 9; }
	}

	// assignment
	{
		c: Color;
		c = .Green;
		if c != Color.Green { return 11; }

		global_color = .Red;
		if global_color != Color.Red { return 12; }
		global_color = .Green;
		if global_color != Color.Green { return 10; }
	}

	// assignment to a struct field
	{
		p: Pixel;
		p.color = .Blue;
		if p.color != Color.Blue { return 13; }
	}

	// assignment through a pointer
	{
		c: Color = .Red;
		pc: *Color = &c;
		*pc = .Green;
		if c != Color.Green { return 14; }

		p: Pixel;
		pp: *Pixel = &p;
		(*pp).color = .Red;
		if p.color != Color.Red { return 15; }
	}

	// assignment to an array element
	{
		arr: [3]Color;
		arr[0] = .Red;
		arr[1] = .Green;
		arr[2] = .Blue;
		if arr[0] != Color.Red   { return 16; }
		if arr[1] != Color.Green { return 17; }
		if arr[2] != Color.Blue  { return 18; }
	}

	// comparison, implicit enumerator on the right
	{
		c: Color = Color.Green;
		if c != .Green { return 19; }
		if c == .Red   { return 20; }
		if !(c == .Green) { return 21; }
	}

	// comparison, implicit enumerator on the left
	{
		c: Color = Color.Blue;
		if .Blue != c { return 22; }
		if .Red == c  { return 23; }
	}

	// function argument
	{
		if to_int(.Red)   != 1 { return 24; }
		if to_int(.Green) != 2 { return 25; }
		if to_int(.Blue)  != 3 { return 26; }
		if second(1, .Blue) != Color.Blue { return 27; }
	}

	// return value
	{
		if next(Color.Red)   != Color.Green { return 28; }
		if next(Color.Green) != Color.Blue  { return 29; }
		if next(Color.Blue)  != Color.Red   { return 30; }
	}

	// switch case labels
	{
		c: Color = .Blue;
		code = 0;
		switch c
		{
			case .Red:   { code = 1; }
			case .Green: { code = 2; }
			case .Blue:  { code = 3; }
		}
		if code != 3 { return 31; }
	}

	// implicit and qualified forms produce the same value
	{
		a: Color = .Green;
		b: Color = Color.Green;
		if a != b { return 33; }
	}

	return 0;
}

to_int :: proc(c: Color) -> i64
{
	if c == .Red   { return 1; }
	if c == .Green { return 2; }
	if c == .Blue  { return 3; }
	return 0;
}

second :: proc(unused: i64, c: Color) -> Color
{
	return c;
}

next :: proc(c: Color) -> Color
{
	switch c
	{
		case .Red:   { return .Green; }
		case .Green: { return .Blue; }
	}
	return .Red;
}

Pixel :: struct
{
	x: i64;
	color: Color;
}
