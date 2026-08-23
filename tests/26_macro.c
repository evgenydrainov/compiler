main :: proc() -> int
{
	array : [..]u64;
	size := sizeof(*array.data);
	print size;
	return 0;
}
