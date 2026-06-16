main :: proc() -> i64
{
    i: i64 = 0;
    sum: i64 = 0;
	while i < 5
	{
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}
