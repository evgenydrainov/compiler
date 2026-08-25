#pragma once

#include "base_types.h"
#include "base_arena.h"
#include "base_slice.h"

template <typename T, usize N>
struct static_bump_array
{
	static constexpr usize capacity = N;

	T data[capacity];
	usize count;

	T &operator[](usize index)
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	const T &operator[](usize index) const
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	T *begin() { return &data[0]; }
	T *end()   { return &data[count]; }
};

template <typename T, usize N>
inline T *
array_add(static_bump_array<T, N> *array, const T &value)
{
	Assert(array->count < array->capacity);

	T *result = &array->data[array->count];

	*result = value;
	array->count++;

	return result;
}

template <typename T, usize N>
inline slice<T>
copy_into_slice(static_bump_array<T, N> array, Arena *arena)
{
	slice<T> result = {};
	result.data = push_array<T>(arena, array.count);
	result.count = array.count;

	MemCpy(result.data, array.data, array.count*sizeof(T));

	return result;
}
