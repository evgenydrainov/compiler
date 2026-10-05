// stackless coroutines driven by coroutine_init/coroutine_resume: locals live
// in a user-provided frame buffer (sized with 'proc.frame_size') and survive
// across yields, data is passed through userdata, early return finishes.
foo :: proc(co: *Coroutine) #coroutine
{
	print 1;
	yield;

	print 2;
	yield;

	print 3;
}

Vec2 :: struct
{
	x: i64;
	y: i64;
}

Counter :: struct
{
	out: i64;
}

counter_data :: macro cast(*Counter)co.userdata;

// locals declared before a yield must keep their values after it
counter :: proc(co: *Coroutine) #coroutine
{
	sum := 0;
	for i in 1..3
	{
		sum += i;
		counter_data.out = sum;
		yield;
	}

	// a struct local and a pointer to a local, both live across a yield
	v: Vec2;
	v.x = 10;
	v.y = 20;
	pv := &v;
	yield;

	pv.x += 1;
	counter_data.out = v.x + v.y + sum;
}

Accumulator :: struct
{
	value: i64;
	out: i64;
}

acc_data :: macro cast(*Accumulator)co.userdata;

// userdata is re-read on every resume
accumulate :: proc(co: *Coroutine) #coroutine
{
	total := 0;
	while true
	{
		total += acc_data.value;
		acc_data.out = total;
		if total >= 100
		{
			return;
		}
		yield;
	}
}

main :: proc() -> i64
{
	{
		buffer: [64]u8;
		if foo.frame_size > buffer.count { return 102; }

		co: Coroutine;
		coroutine_init(&co, foo);

		coroutine_resume(&co, &buffer, null);
		coroutine_resume(&co, &buffer, null);
		if coroutine_is_finished(&co) { return 1; }
		coroutine_resume(&co, &buffer, null);
		if !coroutine_is_finished(&co) { return 2; }
	}

	// two instances of the same coroutine with separate frames, interleaved
	{
		bufferA: [256]u8;
		bufferB: [256]u8;
		if counter.frame_size > bufferA.count { return 103; }

		a: Coroutine;
		b: Coroutine;
		coroutine_init(&a, counter);
		coroutine_init(&b, counter);

		dataA: Counter;
		dataB: Counter;

		coroutine_resume(&a, &bufferA, &dataA);
		if dataA.out != 1 { return 3; }
		coroutine_resume(&a, &bufferA, &dataA);
		if dataA.out != 3 { return 4; }

		coroutine_resume(&b, &bufferB, &dataB);
		if dataB.out != 1 { return 5; }

		coroutine_resume(&a, &bufferA, &dataA);
		if dataA.out != 6 { return 6; }

		coroutine_resume(&a, &bufferA, &dataA); // leaves the loop, sets up v, yields
		if coroutine_is_finished(&a) { return 7; }
		coroutine_resume(&a, &bufferA, &dataA); // finishes
		if dataA.out != 11 + 20 + 6 { return 8; }
		if !coroutine_is_finished(&a) { return 9; }

		// b was not disturbed by a
		coroutine_resume(&b, &bufferB, &dataB);
		if dataB.out != 3 { return 10; }

		// resuming a finished coroutine does nothing
		dataA.out = 0;
		coroutine_resume(&a, &bufferA, &dataA);
		if dataA.out != 0 { return 11; }
	}

	{
		buffer: [256]u8;
		if accumulate.frame_size > buffer.count { return 104; }

		co: Coroutine;
		coroutine_init(&co, accumulate);

		// a different userdata on each resume
		first: Accumulator;
		first.value = 10;
		coroutine_resume(&co, &buffer, &first);
		if first.out != 10 { return 12; }

		second: Accumulator;
		second.value = 50;
		coroutine_resume(&co, &buffer, &second);
		if second.out != 60 { return 13; }
		if first.out != 10 { return 14; }

		first.value = 45;
		coroutine_resume(&co, &buffer, &first);
		if first.out != 105 { return 15; }

		// 'return' inside a coroutine finishes it
		if !coroutine_is_finished(&co) { return 16; }
	}

	return 0;
}
