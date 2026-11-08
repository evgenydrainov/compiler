#include "coroutine.c"
#include "libc.c"
#include "math.c"
#include "vec2.c"
#include "vec3.c"
#include "vec4.c"

string :: struct
{
	data: *u8;
	count: int;
};

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

__range_based_for_expansion_exclusive :: macro(from, to, it, body) #no_bind
{
	it := from;
	__to := to;
	for ; it < __to; it += 1
	{
		body;
	}
}

__range_based_for_expansion_inclusive :: macro(from, to, it, body) #no_bind
{
	it := from;
	__to := to;
	for ; it <= __to; it += 1
	{
		body;
	}
}

__foreach_expansion_by_pointer :: macro(array, it, index, body) #no_bind
{
	// TODO: don't do this copy for static arrays
	__array := array;

	index := 0;
	__count := __array.count;

	for ; index < __count; index += 1;
	{
		it := &array[index];
		body;
	}
}

__foreach_expansion_by_value :: macro(array, it, index, body) #no_bind
{
	// TODO: don't do this copy for static arrays
	__array := array;

	index := 0;
	__count := __array.count;

	for ; index < __count; index += 1;
	{
		it := __array[index];
		body;
	}
}
