main :: proc() -> i64
{
    a: i64 = 10;
    b: i64 = 20;

    pa: *i64 = &a;
    pb: *i64 = &b;

    a = *pb;

    return a;
}
