//asm
//{
//	section .data
//
//	print_int8_format:  db "%hhd", 10, 0
//	print_int16_format: db "%hd",  10, 0
//	print_int32_format: db "%d",   10, 0
//	print_int64_format: db "%lld", 10, 0
//
//	section .text
//}

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

//print_int8 :: proc(value: i8)
//{
//	asm
//	{
//		mov rdx, rcx
//		lea rcx, [rel print_int8_format]
//		sub rsp, 32
//		call printf
//		add rsp, 32
//	}
//}
//
//print_int16 :: proc(value: i16)
//{
//	asm
//	{
//		mov rdx, rcx
//		lea rcx, [rel print_int16_format]
//		sub rsp, 32
//		call printf
//		add rsp, 32
//	}
//}
//
//print_int32 :: proc(value: i32)
//{
//	asm
//	{
//		mov rdx, rcx
//		lea rcx, [rel print_int32_format]
//		sub rsp, 32
//		call printf
//		add rsp, 32
//	}
//}
//
//print_int64 :: proc(value: i64)
//{
//	asm
//	{
//		mov rdx, rcx
//		lea rcx, [rel print_int64_format]
//		sub rsp, 32
//		call printf
//		add rsp, 32
//	}
//}
