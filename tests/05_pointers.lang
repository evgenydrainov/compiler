// address-of, deref read, deref write, pointers as params, aliasing.
main :: proc() -> i64
{
	// read through pointer
	a: i64 = 10;
	b: i64 = 20;
	pa: *i64 = &a;
	pb: *i64 = &b;
	a = *pb;
	if a != 20 { return 1; }

	// write through pointer
	c: i64;
	pc: *i64 = &c;
	*pc = 40;
	if c != 40 { return 2; }

	// mutate original via pointer, observe through name
	d: i64 = 1;
	pd: *i64 = &d;
	*pd = *pd + 41;
	if d != 42 { return 3; }

	// two pointers aliasing same location
	e: i64 = 5;
	p1: *i64 = &e;
	p2: *i64 = &e;
	*p1 = 99;
	if *p2 != 99 { return 4; }

	// pointer parameter round-trip
	f: i64 = 0;
	set_to(&f, 77);
	if f != 77 { return 5; }

	// swap via pointers
	x: i64 = 3;
	y: i64 = 8;
	swap(&x, &y);
	if x != 8 { return 6; }
	if y != 3 { return 7; }

	// pointer to pointer
	g: i64 = 123;
	pg: *i64 = &g;
	ppg: **i64 = &pg;
	if **ppg != 123 { return 8; }
	**ppg = 456;
	if g != 456 { return 9; }

	return 0;
}

set_to :: proc(p: *i64, v: i64) { *p = v; }

swap :: proc(p: *i64, q: *i64)
{
	tmp: i64 = *p;
	*p = *q;
	*q = tmp;
}
