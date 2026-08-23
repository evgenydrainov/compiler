main :: proc() -> int
{
	array : [..]int;
	size := sizeof(*array.data);
	print size;

	p := &array;

	array_add(p, 1);
	array_add(p, 2);
	array_add(p, 3);
	array_add(p, 4);
	array_add(p, 5);

	print p.data;
	print p.count;
	print p.capacity;

	return 0;
}
