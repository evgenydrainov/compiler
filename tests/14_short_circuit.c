hits: int;

bump :: proc(ret: int) -> int
{
	hits += 1;
	return ret;
}

main :: proc() -> int
{
	// && : false LHS should skip RHS
	if bump(0) != 0 && bump(1) != 0
	{
	}
	if hits != 1
	{
		return 1;
	}

	// || : true LHS should skip RHS
	hits = 0;
	if bump(1) != 0 || bump(1) != 0
	{
	}
	if hits != 1
	{
		return 2;
	}

	// && : true LHS must evaluate RHS
	hits = 0;
	if bump(1) != 0 && bump(1) != 0
	{
	}
	if hits != 2
	{
		return 3;
	}

	return 0;
}
