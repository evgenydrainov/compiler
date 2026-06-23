main :: proc() -> i64
{
    p: Point;
    a: i64 = 7;
    p.x = 1;
    p.y = 2;
    return a;
}

Point :: struct
{
	x: i64;
	y: i64;
}
