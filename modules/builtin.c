string :: struct
{
	data: *i8;
	count: i64;
};

CoroutineState :: struct
{
	state: int;
};

int64_to_float32 :: proc(value: i64) -> i32
{
	asm
	{
		cvtsi2ss xmm0, rcx
		movd eax, xmm0
	}
}

_printf__int8  :: proc(format: *u8, value: i8)  #foreign "printf";
_printf__int16 :: proc(format: *u8, value: i16) #foreign "printf";
_printf__int32 :: proc(format: *u8, value: i32) #foreign "printf";
_printf__int64 :: proc(format: *u8, value: i64) #foreign "printf";

print_int8  :: proc(value: i8)  { _printf__int8 ("%hhd\n"c, value); }
print_int16 :: proc(value: i16) { _printf__int16("%hd\n"c,  value); }
print_int32 :: proc(value: i32) { _printf__int32("%d\n"c,   value); }
print_int64 :: proc(value: i64) { _printf__int64("%lld\n"c, value); }
