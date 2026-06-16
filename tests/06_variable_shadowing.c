main :: proc() -> i64
{
    a: i64 = 1;
    {
        a: i64 = 2;
        return a;
    }
}
