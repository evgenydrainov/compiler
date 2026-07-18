// calls, argument order, nesting, recursion, void procs, early return.
main :: proc() -> i64
{
	if add(40, 2) != 42 { return 1; }

	// argument order must be preserved (non-commutative op)
	if sub(10, 3) != 7 { return 2; }
	if sub(3, 10) != -7 { return 3; }

	// nested calls / evaluation order
	if add(add(1, 2), add(3, 4)) != 10 { return 4; }
	if sub(sub(20, 5), sub(8, 3)) != 10 { return 5; }

	// recursion
	if factorial(5) != 120 { return 6; }
	if factorial(0) != 1 { return 7; }
	if fib(10) != 55 { return 8; }

	// mutual recursion
	if is_even(10) != 1 { return 9; }
	if is_even(7)  != 0 { return 10; }

	// many arguments, order check via weighted sum
	if weighted(1, 2, 3, 4) != 1234 { return 11; }

	// void proc with side effect through pointer
	acc: i64 = 0;
	accumulate(&acc, 5);
	accumulate(&acc, 7);
	if acc != 12 { return 12; }

	// early return short-circuits
	if pick(1) != 111 { return 13; }
	if pick(0) != 222 { return 14; }

	return 0;
}

add :: proc(a: i64, b: i64) -> i64 { return a + b; }
sub :: proc(a: i64, b: i64) -> i64 { return a - b; }

factorial :: proc(n: i64) -> i64
{
	if n <= 1 { return 1; }
	return n * factorial(n - 1);
}

fib :: proc(n: i64) -> i64
{
	if n < 2 { return n; }
	return fib(n - 1) + fib(n - 2);
}

is_even :: proc(n: i64) -> i64
{
	if n == 0 { return 1; }
	return is_odd(n - 1);
}

is_odd :: proc(n: i64) -> i64
{
	if n == 0 { return 0; }
	return is_even(n - 1);
}

weighted :: proc(a: i64, b: i64, c: i64, d: i64) -> i64
{
	return a * 1000 + b * 100 + c * 10 + d;
}

accumulate :: proc(acc: *i64, v: i64)
{
	*acc = *acc + v;
}

pick :: proc(flag: i64) -> i64
{
	if flag != 0 {
		return 111;
	}
	return 222;
}
