fminf  :: proc(a: f32, b: f32) -> f32 #foreign;
fmaxf  :: proc(a: f32, b: f32) -> f32 #foreign;
sqrtf  :: proc(x: f32)         -> f32 #foreign;

printf :: proc(format: *u8) #foreign #variadic;

rand :: proc() -> i32 #foreign;

malloc  :: proc(size: int)             -> *void #foreign;
calloc  :: proc(count: int, size: int) -> *void #foreign;
realloc :: proc(ptr: *void, size: int) -> *void #foreign;
free    :: proc(ptr: *void)                     #foreign;

fabsf :: proc(a: f32) -> f32
{
	if a >= 0 return a;
	return -a;
}
