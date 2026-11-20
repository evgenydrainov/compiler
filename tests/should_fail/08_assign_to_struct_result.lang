main :: proc() -> i64
{
	makeV2(1, 2).x = 5;
	return 0;
}

makeV2 :: proc(x: i64, y: i64) -> V2
{
	v: V2;
	v.x = x;
	v.y = y;
	return v;
}

V2 :: struct
{
	x: i64;
	y: i64;
}
