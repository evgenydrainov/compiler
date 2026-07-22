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
fminf :: proc(a: f32, b: f32) -> f32 #foreign;
fmaxf :: proc(a: f32, b: f32) -> f32 #foreign;
