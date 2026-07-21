string :: struct
{
	data: *u8;
	count: i64;
};

CoroutineState :: struct
{
	state: i64;
};

printf :: proc(format: *u8) #foreign #variadic;
