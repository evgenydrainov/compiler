main :: proc() -> i64
{
    sum: i64 = 0;
    for i: i64 = 0; i < 5; i = i + 1;
    {
        sum = sum + i;
    }

    return sum;
}
