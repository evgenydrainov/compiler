// Foreign function interface: call into C runtime (putchar). Prints "Hello World".
main :: proc() -> i64
{
	print_word();
	putchar(32);
	print_world();
	putchar(10);
	return 0;
}

print_word :: proc()
{
	putchar(72);   // H
	putchar(101);  // e
	putchar(108);  // l
	putchar(108);  // l
	putchar(111);  // o
}

print_world :: proc()
{
	putchar(87);   // W
	putchar(111);  // o
	putchar(114);  // r
	putchar(108);  // l
	putchar(100);  // d
}

putchar :: proc(c: i64) -> i64 #foreign;
