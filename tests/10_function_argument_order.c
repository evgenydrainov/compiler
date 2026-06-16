main :: proc() -> i64
{
    return sub(10, 3);
}

sub :: proc(a: i64, b: i64) -> i64
{
    return a - b;
}
