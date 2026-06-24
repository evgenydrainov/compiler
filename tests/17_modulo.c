main :: proc() -> i64
{
    if 56 % 10 != 6
    {
        return 1;
    }

    if -56 % 10 != -6
    {
        return 2;
    }

    return 0;
}
