Coroutine_Proc :: #type proc(co: *Coroutine) #coroutine;

// VOLATILE: struct layout must match the compiler implementation
Coroutine :: struct
{
	state: int;
	frame: *void;
	func: Coroutine_Proc;
	userdata: *void;
};

coroutine_init :: proc(co: *Coroutine, func: Coroutine_Proc)
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
