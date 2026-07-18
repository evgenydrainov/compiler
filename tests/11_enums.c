// enum values, equality, assignment, use in branches and comparisons.
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

	return 0;
}

Color :: enum
{
	Red;
	Green;
	Blue;
};
