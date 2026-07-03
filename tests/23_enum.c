main :: proc() -> i64
{
	color: Color;

	color = Color.Red;
	if color != Color.Red
	{
		return 1;
	}

	color = Color.Green;
	if color != Color.Green
	{
		return 1;
	}

	color = Color.Blue;
	if color != Color.Blue
	{
		return 1;
	}

	return 0;
}

Color :: enum
{
	Red;
	Green;
	Blue;
};
