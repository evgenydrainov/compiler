string :: struct
{
	data: *u8;
	count: int;
};

Coroutine :: struct
{
	state: int;
	userdata: *void;
	i: int;
	j: int;
	k: int;
	l: int;
};

fminf  :: proc(a: f32, b: f32) -> f32 #foreign;
fmaxf  :: proc(a: f32, b: f32) -> f32 #foreign;
sqrtf  :: proc(x: f32)         -> f32 #foreign;

printf :: proc(format: *u8) #foreign #variadic;

rand :: proc() -> i32 #foreign;

malloc  :: proc(size: int)             -> *void #foreign;
calloc  :: proc(count: int, size: int) -> *void #foreign;
realloc :: proc(ptr: *void, size: int) -> *void #foreign;
free    :: proc(ptr: *void)                     #foreign;

fabsf :: proc(a: f32) -> f32
{
	if a >= 0.0
	{
		return a;
	}
	else
	{
		return -a;
	}
}

Raw_Dynamic_Array :: struct
{
	data     : *void;
	count    : int;
	capacity : int;
};

__array_reserve :: proc(array: *Raw_Dynamic_Array, elem_size: int, want_capacity: int)
{
	if want_capacity <= array.capacity
	{
		return;
	}

	new_capacity := array.capacity;
	if new_capacity == 0
	{
		new_capacity = 8;
	}

	while new_capacity < want_capacity
	{
		new_capacity *= 2;
	}

	array.data = realloc(array.data, new_capacity*elem_size);
	array.capacity = new_capacity;
}

__array_free :: proc(array: *Raw_Dynamic_Array)
{
	free(array.data);
	array.data = null;
	array.count = 0;
	array.capacity = 0;
}

array_add :: macro(array, value)
{
	__array_reserve(cast(*Raw_Dynamic_Array)array, sizeof(*array.data), array.count+1);
	array.data[array.count] = value;
	array.count += 1;
}

array_reserve :: macro(array, want_capacity)
{
	__array_reserve(cast(*Raw_Dynamic_Array)array, sizeof(*array.data), want_capacity);
}

array_free :: macro(array)
{
	__array_free(cast(*Raw_Dynamic_Array)array);
}

array_unordered_remove :: macro(array, index)
{
	if index < array.count-1
	{
		array.data[index] = array.data[array.count-1];
	}
	array.count -= 1;
}
