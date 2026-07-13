main :: proc() -> i64
{
	arr: [5]i64;

	arr[0] = 10;
	arr[1] = 20;
	arr[2] = 30;
	arr[3] = 40;
	arr[4] = 50;

	if arr[0] + arr[1] + arr[2] + arr[3] + arr[4] != 150
	{
		return 1;
	}

	i: i64 = 2;
	if arr[i] != 30
	{
		return 2;
	}

	arr[i] = arr[i] + 5;
	if arr[2] != 35
	{
		return 3;
	}

	return 0;
}
