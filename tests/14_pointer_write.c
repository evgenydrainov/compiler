main :: proc() -> i64
{
    a: i64;
    pa: *i64 = &a;

    *pa = 40;

    return a;
}
