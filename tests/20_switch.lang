// switch: dispatch, default, no implicit fallthrough,
// enum subjects, scoping, and interaction with loops, defer and return.

seq: int;
calls: int;

record :: proc(v: int)
{
	seq = seq*10 + v;
}

reset :: proc()
{
	seq = 0;
}

// returns its argument and counts how many times it was called
probe :: proc(v: int) -> int
{
	calls = calls + 1;
	return v;
}

LIMIT :: 5;

// every path returns from inside a case
classify :: proc(n: int) -> int
{
	switch n
	{
		case 0: { return 100; }
		case 1: { return 200; }
		case 2: { return 200; }
		case 3: { return 200; }
		default: { return 300; }
	}

	return 400;
}

// a return out of a case still flushes the defers around it
guarded :: proc(n: int) -> int
{
	defer record(1);

	switch n
	{
		case 1:
		{
			defer record(2);
			return 5;
		}
		default:
		{
			return 6;
		}
	}

	return 7;
}

main :: proc() -> int
{
	// 1. each label selects its own body
	if classify(0)  != 100 { return 1; }
	if classify(1)  != 200 { return 2; }
	if classify(2)  != 200 { return 3; }
	if classify(3)  != 200 { return 4; }
	if classify(7)  != 300 { return 5; }
	if classify(-1) != 300 { return 6; }

	// 2. no implicit fallthrough - only the matching body runs
	reset();
	switch 1
	{
		case 1: { record(1); }
		case 2: { record(2); }
		default: { record(3); }
	}
	if seq != 1 { return 7; }

	// 3. default runs only when nothing matched
	reset();
	switch 99
	{
		case 1: { record(1); }
		case 2: { record(2); }
		default: { record(3); }
	}
	if seq != 3 { return 8; }

	// 4. no default and no match - the whole switch is a no-op
	reset();
	record(5);
	switch 42
	{
		case 1: { record(1); }
		case 2: { record(2); }
	}
	if seq != 5 { return 9; }

	// 5. an empty case body is still a match (the default must not run)
	reset();
	switch 1
	{
		case 1: { }
		default: { record(9); }
	}
	if seq != 0 { return 10; }

	// 6. the subject is evaluated exactly once
	calls = 0;
	reset();
	switch probe(2)
	{
		case 1: { record(1); }
		case 2: { record(2); }
		default: { record(3); }
	}
	if seq != 2 { return 11; }
	if calls != 1 { return 12; }

	// 7. the subject is evaluated even when nothing matches
	calls = 0;
	switch probe(77)
	{
		case 1: { record(1); }
	}
	if calls != 1 { return 13; }

	// 8. an arbitrary expression as the subject
	out := 0;
	v := 3;
	switch v * 2
	{
		case 6: { out = 60; }
		default: { out = -1; }
	}
	if out != 60 { return 14; }

	// 9. a named constant as a label
	reset();
	switch 5
	{
		case LIMIT: { record(1); }
		default: { record(2); }
	}
	if seq != 1 { return 15; }

	// 10. constant-folded expressions as labels
	reset();
	switch 6
	{
		case 2 + 3: { record(1); }
		case 2 * 3: { record(2); }
		default: { record(3); }
	}
	if seq != 2 { return 16; }

	// 11. negative labels
	reset();
	switch -2
	{
		case -1: { record(1); }
		case -2: { record(2); }
		default: { record(3); }
	}
	if seq != 2 { return 17; }

	// 12. a denser run of labels still picks exactly one
	reset();
	switch 3
	{
		case 1: { record(1); }
		case 2: { record(2); }
		case 3: { record(3); }
		case 4: { record(4); }
		default: { record(9); }
	}
	if seq != 3 { return 18; }

	// 13. enum subject with qualified labels
	color: Color;

	color = Color.Red;
	code := 0;
	switch color
	{
		case Color.Red: { code = 1; }
		case Color.Green: { code = 2; }
		case Color.Blue: { code = 3; }
	}
	if code != 1 { return 19; }

	color = Color.Green;
	code = 0;
	switch color
	{
		case Color.Red: { code = 1; }
		case Color.Green: { code = 2; }
		case Color.Blue: { code = 3; }
	}
	if code != 2 { return 20; }

	color = Color.Blue;
	code = 0;
	switch color
	{
		case Color.Red: { code = 1; }
		case Color.Green: { code = 2; }
		case Color.Blue: { code = 3; }
	}
	if code != 3 { return 21; }

	// 14. enum subject with implicit member labels
	color = Color.Green;
	code = 0;
	switch color
	{
		case .Red: { code = 1; }
		case .Green: { code = 2; }
		case .Blue: { code = 3; }
		default: { code = 9; }
	}
	if code != 2 { return 22; }

	// 15. a case body is its own scope
	reset();
	switch 1
	{
		case 1: { n := 4; record(n); }
		case 2: { n := 5; record(n); }
	}
	if seq != 4 { return 23; }

	// 16. a declaration in a case shadows, it does not leak
	shadow := 8;
	switch 1
	{
		case 1: { shadow := 1; record(shadow); }
	}
	if shadow != 8 { return 24; }

	// 17. nested switch
	reset();
	switch 1
	{
		case 1:
		{
			switch 2
			{
				case 1: { record(1); }
				case 2: { record(2); }
			}
			record(3);
		}
		case 2: { record(9); }
	}
	if seq != 23 { return 25; }

	// 18. break inside a case still breaks the enclosing loop
	reset();
	for i := 0; i < 5; i += 1
	{
		switch i
		{
			case 3: { break; }
			default: { record(1); }
		}
	}
	if seq != 111 { return 26; }

	// 19. continue inside a case still continues the enclosing loop
	reset();
	for i := 0; i < 4; i += 1
	{
		switch i
		{
			case 1: { continue; }
			case 2: { record(2); }
			default: { record(9); }
		}
		record(3);
	}
	if seq != 932393 { return 27; }

	// 20. a switch in a while loop
	reset();
	w := 0;
	while 1 == 1
	{
		w += 1;
		switch w
		{
			case 3: { break; }
			default: { record(1); }
		}
	}
	if w != 3 { return 28; }
	if seq != 11 { return 29; }

	// 21. a defer inside a case flushes when the case body ends
	reset();
	switch 1
	{
		case 1: { defer record(1); record(2); }
		default: { record(9); }
	}
	if seq != 21 { return 30; }

	// 22. returning out of a case flushes the defers around it
	reset();
	g := guarded(1);
	if g != 5 { return 31; }
	if seq != 21 { return 32; }

	reset();
	g = guarded(9);
	if g != 6 { return 33; }
	if seq != 1 { return 34; }

	return 0;
}

Color :: enum
{
	Red;
	Green;
	Blue;
};
