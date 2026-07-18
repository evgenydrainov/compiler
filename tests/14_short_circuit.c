bump :: proc(ret: i64, hits: *i64) -> i64
{
	*hits += 1;
	return ret;
}

main :: proc() -> i64
{
	// && : false LHS should skip RHS
	hits: i64 = 0;
	if bump(0, &hits) != 0 && bump(1, &hits) != 0 {
	}
	if hits != 1 { return 1; }   // RHS must not have run

	// || : true LHS should skip RHS
	hits = 0;
	if bump(1, &hits) != 0 || bump(1, &hits) != 0 {
	}
	if hits != 1 { return 2; }

	// && : true LHS must evaluate RHS
	hits = 0;
	if bump(1, &hits) != 0 && bump(1, &hits) != 0 {
	}
	if hits != 2 { return 3; }

	return 0;
}
