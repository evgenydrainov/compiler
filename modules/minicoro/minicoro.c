#link_library "minicoro_x86-64_vs2022_mt.lib";

/* Coroutine states. */
mco_state :: enum i32 {
	DEAD :: 0;  /* The coroutine has finished normally or was uninitialized before finishing. */
	NORMAL;     /* The coroutine is active but not running (that is, it has resumed another coroutine). */
	RUNNING;    /* The coroutine is active and running. */
	SUSPENDED;  /* The coroutine is suspended (in a call to yield, or it has not started running yet). */
};

/* Coroutine result codes. */
mco_result :: enum i32 {
	SUCCESS :: 0;
	GENERIC_ERROR;
	INVALID_POINTER;
	INVALID_COROUTINE;
	NOT_SUSPENDED;
	NOT_RUNNING;
	MAKE_CONTEXT_ERROR;
	SWITCH_CONTEXT_ERROR;
	NOT_ENOUGH_SPACE;
	OUT_OF_MEMORY;
	INVALID_ARGUMENTS;
	INVALID_OPERATION;
	STACK_OVERFLOW;
};

/* Coroutine structure. */
mco_coro :: struct {
	context         : *void;
	state           : mco_state;
	func            : proc(co: *mco_coro);
	prev_co         : *mco_coro;
	user_data       : *void;
	coro_size       : size_t;
	allocator_data  : *void;
	dealloc_cb      : proc(ptr: *void, size: size_t, allocator_data: *void);
	stack_base      : *void; /* Stack base address, can be used to scan memory in a garbage collector. */
	stack_size      : size_t;
	storage         : *u8;
	bytes_stored    : size_t;
	storage_size    : size_t;
	asan_prev_stack : *void; /* Used by address sanitizer. */
	tsan_prev_fiber : *void; /* Used by thread sanitizer. */
	tsan_fiber      : *void; /* Used by thread sanitizer. */
	magic_number    : size_t; /* Used to check stack overflow. */
};

/* Structure used to initialize a coroutine. */
mco_desc :: struct {
	func           : proc(co: *mco_coro); /* Entry point function for the coroutine. */
	user_data      : *void;               /* Coroutine user data, can be get with `mco_get_user_data`. */
	/* Custom allocation interface. */
	alloc_cb       : proc(size: size_t, allocator_data: *void) -> *void;    /* Custom allocation function. */
	dealloc_cb     : proc(ptr: *void, size: size_t, allocator_data: *void); /* Custom deallocation function. */
	allocator_data : *void;      /* User data pointer passed to `alloc`/`dealloc` allocation functions. */
	storage_size   : size_t;     /* Coroutine storage size, to be used with the storage APIs. */
	/* These must be initialized only through `mco_init_desc`. */
	coro_size      : size_t;        /* Coroutine structure size. */
	stack_size     : size_t;        /* Coroutine stack size. */
};

/* Coroutine functions. */
mco_desc_init     :: proc(func: proc(co: *mco_coro), stack_size: size_t) -> mco_desc   #foreign; /* Initialize description of a coroutine. When stack size is 0 then MCO_DEFAULT_STACK_SIZE is used. */
mco_init          :: proc(co: *mco_coro, desc: *mco_desc)                -> mco_result #foreign; /* Initialize the coroutine. */
mco_uninit        :: proc(co: *mco_coro)                                 -> mco_result #foreign; /* Uninitialize the coroutine, may fail if it's not dead or suspended. */
mco_create        :: proc(out_co: **mco_coro, desc: *mco_desc)           -> mco_result #foreign; /* Allocates and initializes a new coroutine. */
mco_destroy       :: proc(co: *mco_coro)                                 -> mco_result #foreign; /* Uninitialize and deallocate the coroutine, may fail if it's not dead or suspended. */
mco_resume        :: proc(co: *mco_coro)                                 -> mco_result #foreign; /* Starts or continues the execution of the coroutine. */
mco_yield         :: proc(co: *mco_coro)                                 -> mco_result #foreign; /* Suspends the execution of a coroutine. */
mco_status        :: proc(co: *mco_coro)                                 -> mco_state  #foreign; /* Returns the status of the coroutine. */
mco_get_user_data :: proc(co: *mco_coro)                                 -> *void      #foreign; /* Get coroutine user data supplied on coroutine creation. */

/* Storage interface functions, used to pass values between yield and resume. */
mco_push :: proc(co: *mco_coro, src: *void, len: size_t)  -> mco_result #foreign; /* Push bytes to the coroutine storage. Use to send values between yield and resume. */
mco_pop  :: proc(co: *mco_coro, dest: *void, len: size_t) -> mco_result #foreign; /* Pop bytes from the coroutine storage. Use to get values between yield and resume. */
mco_peek :: proc(co: *mco_coro, dest: *void, len: size_t) -> mco_result #foreign; /* Like `mco_pop` but it does not consumes the storage. */
mco_get_bytes_stored :: proc(co: *mco_coro) -> size_t #foreign;                   /* Get the available bytes that can be retrieved with a `mco_pop`. */
mco_get_storage_size :: proc(co: *mco_coro) -> size_t #foreign;                   /* Get the total storage size. */

/* Misc functions. */
mco_running :: proc() -> *mco_coro #foreign;                     /* Returns the running coroutine for the current thread. */
mco_result_description :: proc(res: mco_result) -> *u8 #foreign; /* Get the description of a result. */
