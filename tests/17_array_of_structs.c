// arrays of structs, and taking address of an array element.
main :: proc() -> i64
{
	pts: [3]Point;

	pts[0].x = 1; pts[0].y = 2;
	pts[1].x = 3; pts[1].y = 4;
	pts[2].x = 5; pts[2].y = 6;

	total: i64 = 0;
	for i: i64 = 0; i < 3; i = i + 1 {
		total = total + pts[i].x + pts[i].y;
	}
	if total != 21 { return 1; }

	// address of an element, mutate through pointer
	p: *Point = &pts[1];
	(*p).x = 100;
	if pts[1].x != 100 { return 2; }

	p.x = 200;
	if pts[1].x != 200 { return 3; }

	return 0;
}

Point :: struct
{
	x: i64;
	y: i64;
}
