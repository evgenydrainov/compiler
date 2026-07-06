string :: struct
{
	data: *i8;
	count: i64;
};

int64_to_float32 :: proc(value: i64) -> i32
{
	asm
	{
		cvtsi2ss xmm0, rcx
		movd eax, xmm0
	}
}
