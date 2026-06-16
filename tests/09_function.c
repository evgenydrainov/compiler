main :: proc() -> i64
{
    return add(40, 2);
}

add :: proc(a: i64, b: i64) -> i64
{
    return a + b;
}
