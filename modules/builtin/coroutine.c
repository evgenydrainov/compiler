Coroutine :: struct
{
	// has to be at offset 0
	state: int;

	// has to be at offset 8
	frame: *void;

	func: proc(co: *Coroutine) #coroutine;
	
	userdata: *void;
};

coroutine_init :: proc(co: *Coroutine,
					   func: proc(co: *Coroutine) #coroutine)
{
	co.func = func;
	co.state = 0;
}

coroutine_resume :: proc(co: *Coroutine, frame: *void, userdata: *void)
{
	co.frame = frame;
	co.userdata = userdata;
	co.func(co);
}

coroutine_is_finished :: proc(co: *Coroutine) -> bool
{
	return co.state == -1;
}
