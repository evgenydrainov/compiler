// SHOULD FAIL: taking the address of an element of an array that has no
// address of its own.
main :: proc() -> i64
{
	s := make_slice_i64(&makeBag().items[0], 3);
	return s.count;
}

makeBag :: proc() -> Bag
{
	bag: Bag;
	bag.items[0] = 1;
	bag.items[1] = 2;
	bag.items[2] = 3;
	return bag;
}

make_slice_i64 :: proc(data: *i64, count: i64) -> []i64
{
	s: []i64;
	s.data  = data;
	s.count = count;
	return s;
}

Bag :: struct
{
	items: [3]i64;
}
