// if/else, else-if chains, while, for, nested loops, single-stmt bodies.
main :: proc() -> i64
{
	// if / else
	a: i64 = 7;
	r: i64 = 0;
	if a < 10 { r = 1; } else { r = 2; }
	if r != 1 { return 1; }

	// else-if chain, all branches
	if classify(-5) != -1 { return 2; }
	if classify(0)  !=  0 { return 3; }
	if classify(9)  !=  1 { return 4; }

	// single-statement bodies (no braces)
	b: i64 = 3;
	if b == 3
		r = 100;
	else
		r = 200;
	if r != 100 { return 5; }

	// while loop accumulation
	i: i64 = 0;
	sum: i64 = 0;
	while i < 5 {
		sum = sum + i;
		i = i + 1;
	}
	if sum != 10 { return 6; }

	// while that never executes
	count: i64 = 0;
	while 0 > 1 {
		count = count + 1;
	}
	if count != 0 { return 7; }

	// for loop
	fsum: i64 = 0;
	for j: i64 = 0; j < 5; j = j + 1 {
		fsum = fsum + j;
	}
	if fsum != 10 { return 8; }

	// nested loops: 3x3 grid count
	cells: i64 = 0;
	for p: i64 = 0; p < 3; p = p + 1 {
		for q: i64 = 0; q < 3; q = q + 1 {
			cells = cells + 1;
		}
	}
	if cells != 9 { return 9; }

	// loop-carried multiplication (factorial-ish, iterative)
	fact: i64 = 1;
	for k: i64 = 1; k <= 5; k = k + 1 {
		fact = fact * k;
	}
	if fact != 120 { return 10; }

	return 0;
}

classify :: proc(n: i64) -> i64
{
	if n < 0
		return -1;
	else if n > 0
		return 1;

	return 0;
}
