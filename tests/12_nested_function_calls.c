main :: proc() -> i64
{
    return add(add(1, 2), add(3, 4));
}

add :: proc(a: i64, b: i64) -> i64
{
    return a + b;
}
