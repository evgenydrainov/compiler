// nested structs, struct containing array, and multi-dimensional arrays.
main :: proc() -> i64
{
	// struct containing a struct
	{
		r: Rect;
		r.min.x = 1;
		r.min.y = 2;
		r.max.x = 4;
		r.max.y = 6;
		if (r.max.x - r.min.x) != 3 { return 1; }
		if (r.max.y - r.min.y) != 4 { return 2; }
	}

	// struct containing an array field
	{
		bag: Bag;
		bag.items[0] = 10;
		bag.items[1] = 20;
		bag.items[2] = 30;
		bag.count = 3;
		if bag.items[0] + bag.items[1] + bag.items[2] != 60 { return 3; }
		if bag.count != 3 { return 4; }
	}

	// 2D array
	{
		grid: [3][3]i64;
		for i := 0; i < 3; i += 1 {
			for j := 0; j < 3; j += 1 {
				grid[i][j] = i * 3 + j;
			}
		}
		if grid[0][0] != 0 { return 5; }
		if grid[2][2] != 8 { return 6; }
		if grid[1][2] != 5 { return 7; }
	}

	return 0;
}

Point :: struct
{
	x: i64;
	y: i64;
}

Rect :: struct
{
	min: Point;
	max: Point;
}

Bag :: struct
{
	items: [4]i64;
	count: i64;
}
