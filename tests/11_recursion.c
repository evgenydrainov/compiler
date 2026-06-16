main :: proc() -> i64
{
    return factorial(5);
}

factorial :: proc(n: i64) -> i64
{
    if n <= 1
	{
        return 1;
    }
    return n * factorial(n - 1);
}
