/* Coroutine states. */
MCO_DEAD      :: 0;  /* The coroutine has finished normally or was uninitialized before finishing. */
MCO_NORMAL    :: 1;  /* The coroutine is active but not running (that is, it has resumed another coroutine). */
MCO_RUNNING   :: 2;  /* The coroutine is active and running. */
MCO_SUSPENDED :: 3;  /* The coroutine is suspended (in a call to yield, or it has not started running yet). */

/* Coroutine result codes. */
MCO_SUCCESS              :: 0;
MCO_GENERIC_ERROR        :: 1;
MCO_INVALID_POINTER      :: 2;
MCO_INVALID_COROUTINE    :: 3;
MCO_NOT_SUSPENDED        :: 4;
MCO_NOT_RUNNING          :: 5;
MCO_MAKE_CONTEXT_ERROR   :: 6;
MCO_SWITCH_CONTEXT_ERROR :: 7;
MCO_NOT_ENOUGH_SPACE     :: 8;
MCO_OUT_OF_MEMORY        :: 9;
MCO_INVALID_ARGUMENTS    :: 10;
MCO_INVALID_OPERATION    :: 11;
MCO_STACK_OVERFLOW       :: 12;

/* Coroutine structure. */
mco_coro :: struct {
	context         : *void;
	state           : i32;
	func            : proc(*mco_coro);
	prev_co         : *mco_coro;
	user_data       : *void;
	coro_size       : u64;
	allocator_data  : *void;
	dealloc_cb      : proc(*void, u64, *void);
	stack_base      : *void; /* Stack base address, can be used to scan memory in a garbage collector. */
	stack_size      : u64;
	storage         : *u8;
	bytes_stored    : u64;
	storage_size    : u64;
	asan_prev_stack : *void; /* Used by address sanitizer. */
	tsan_prev_fiber : *void; /* Used by thread sanitizer. */
	tsan_fiber      : *void; /* Used by thread sanitizer. */
	magic_number    : u64; /* Used to check stack overflow. */
};

/* Structure used to initialize a coroutine. */
mco_desc :: struct {
	func           : proc(*mco_coro); /* Entry point function for the coroutine. */
	user_data      : *void;           /* Coroutine user data, can be get with `mco_get_user_data`. */
	/* Custom allocation interface. */
	alloc_cb       : proc(u64, *void) -> *void; /* Custom allocation function. */
	dealloc_cb     : proc(*void, u64, *void);   /* Custom deallocation function. */
	allocator_data : *void;      /* User data pointer passed to `alloc`/`dealloc` allocation functions. */
	storage_size   : u64;        /* Coroutine storage size, to be used with the storage APIs. */
	/* These must be initialized only through `mco_init_desc`. */
	coro_size      : u64;        /* Coroutine structure size. */
	stack_size     : u64;        /* Coroutine stack size. */
};

/* Coroutine functions. */
mco_desc_init     :: proc(func: proc(*mco_coro), stack_size: u64) -> mco_desc #foreign; /* Initialize description of a coroutine. When stack size is 0 then MCO_DEFAULT_STACK_SIZE is used. */
mco_init          :: proc(co: *mco_coro, desc: *mco_desc)         -> i32      #foreign; /* Initialize the coroutine. */
mco_uninit        :: proc(co: *mco_coro)                          -> i32      #foreign; /* Uninitialize the coroutine, may fail if it's not dead or suspended. */
mco_create        :: proc(out_co: **mco_coro, desc: *mco_desc)    -> i32      #foreign; /* Allocates and initializes a new coroutine. */
mco_destroy       :: proc(co: *mco_coro)                          -> i32      #foreign; /* Uninitialize and deallocate the coroutine, may fail if it's not dead or suspended. */
mco_resume        :: proc(co: *mco_coro)                          -> i32      #foreign; /* Starts or continues the execution of the coroutine. */
mco_yield         :: proc(co: *mco_coro)                          -> i32      #foreign; /* Suspends the execution of a coroutine. */
mco_status        :: proc(co: *mco_coro)                          -> i32      #foreign; /* Returns the status of the coroutine. */
mco_get_user_data :: proc(co: *mco_coro)                          -> *void    #foreign; /* Get coroutine user data supplied on coroutine creation. */

/* Storage interface functions, used to pass values between yield and resume. */
//MCO_API mco_result mco_push(mco_coro* co, const void* src, size_t len); /* Push bytes to the coroutine storage. Use to send values between yield and resume. */
//MCO_API mco_result mco_pop(mco_coro* co, void* dest, size_t len);       /* Pop bytes from the coroutine storage. Use to get values between yield and resume. */
//MCO_API mco_result mco_peek(mco_coro* co, void* dest, size_t len);      /* Like `mco_pop` but it does not consumes the storage. */
//MCO_API size_t mco_get_bytes_stored(mco_coro* co);                      /* Get the available bytes that can be retrieved with a `mco_pop`. */
//MCO_API size_t mco_get_storage_size(mco_coro* co);                      /* Get the total storage size. */

/* Misc functions. */
//MCO_API mco_coro* mco_running(void);                        /* Returns the running coroutine for the current thread. */
//MCO_API const char* mco_result_description(mco_result res); /* Get the description of a result. */
