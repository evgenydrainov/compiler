// array store/load, constant and variable index, index expressions, iteration.
main :: proc() -> i64
{
	arr: [5]i64;
	arr[0] = 10;
	arr[1] = 20;
	arr[2] = 30;
	arr[3] = 40;
	arr[4] = 50;

	if arr[0] + arr[1] + arr[2] + arr[3] + arr[4] != 150 { return 1; }

	// variable index read
	i: i64 = 2;
	if arr[i] != 30 { return 2; }

	// read-modify-write at variable index
	arr[i] = arr[i] + 5;
	if arr[2] != 35 { return 3; }

	// index by expression
	if arr[1 + 1] != 35 { return 4; }
	if arr[i - 2] != 10 { return 5; }

	// fill and sum by loop
	nums: [10]i64;
	for k: i64 = 0; k < 10; k = k + 1 {
		nums[k] = k * k;
	}
	total: i64 = 0;
	for k: i64 = 0; k < 10; k = k + 1 {
		total = total + nums[k];
	}
	if total != 285 { return 6; }

	// first and last element boundaries
	if nums[0] != 0 { return 7; }
	if nums[9] != 81 { return 8; }

	// use one array element to index another
	idx: [3]i64;
	idx[0] = 4;
	if arr[idx[0]] != 50 { return 9; }

	return 0;
}
