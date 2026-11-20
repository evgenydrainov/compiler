seq: int;

record :: proc(v: int)
{
	seq = seq*10 + v;
}

reset :: proc()
{
	seq = 0;
}

// falls off the end with no explicit return
fall_off :: proc()
{
	defer record(1);
	record(2);
}

// an early return must still flush the defers above it, in reverse
early_return :: proc() -> int
{
	defer record(1);
	defer record(2);

	if 1 == 1
	{
		record(3);
		return 7;
	}

	record(9);
	return 0;
}

// a return from an inner scope flushes the inner defers, then the outer ones
nested_return :: proc() -> int
{
	defer record(1);

	{
		defer record(2);
		defer record(3);
		return 5;
	}

	return 0;
}

main :: proc() -> int
{
	// 1. LIFO within a scope, after the normal statements have run
	reset();
	{
		defer record(1);
		defer record(2);
		record(3);
	}
	if seq != 321
	{
		return 1;
	}

	// 2. an inner scope flushes at the inner brace, not the outer one
	reset();
	{
		defer record(1);
		{
			defer record(2);
			record(3);
		}
		record(4);
	}
	if seq != 3241
	{
		return 2;
	}

	// 3. a defer whose body is a block runs its statements in order
	reset();
	{
		defer { record(1); record(2); }
		record(3);
	}
	if seq != 312
	{
		return 3;
	}

	// 4. falling off the end of a proc runs its defers
	reset();
	fall_off();
	if seq != 21
	{
		return 4;
	}

	// 5. early return: the value still comes back, defers still run in reverse
	reset();
	r := early_return();
	if r != 7
	{
		return 5;
	}
	if seq != 321
	{
		return 6;
	}

	// 6. return from a nested scope flushes inner defers first, then outer
	reset();
	r2 := nested_return();
	if r2 != 5
	{
		return 7;
	}
	if seq != 321
	{
		return 8;
	}

	// 7. a defer in a loop body runs once per iteration
	reset();
	for i := 0; i < 3; i += 1
	{
		defer record(1);
	}
	if seq != 111
	{
		return 9;
	}

	// 8. break flushes the loop-body defers, but not the enclosing scope's
	reset();
	{
		defer record(4);
		for i := 0; i < 5; i += 1
		{
			defer record(1);
			if i == 1
			{
				break;
			}
		}
		record(9);
	}
	if seq != 1194
	{
		return 10;
	}

	// 9. continue flushes the loop-body defers too
	reset();
	for i := 0; i < 3; i += 1
	{
		defer record(1);
		if i % 2 == 0
		{
			continue;
		}
		record(2);
	}
	if seq != 1211
	{
		return 11;
	}

	// 10. break out of a while loop
	reset();
	w := 0;
	while 1 == 1
	{
		defer record(1);
		w += 1;
		if w == 2
		{
			break;
		}
	}
	if seq != 11
	{
		return 12;
	}

	// 11. a deferred statement observes the value at scope exit, not at 'defer'
	reset();
	{
		x := 5;
		defer record(x);
		x = 7;
	}
	if seq != 7
	{
		return 13;
	}

	// 12. defers do not fire early - nothing has run yet mid-scope
	reset();
	{
		defer record(1);
		record(2);
		if seq != 2
		{
			return 14;
		}
	}
	if seq != 21
	{
		return 15;
	}

	return 0;
}
