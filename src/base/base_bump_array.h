#pragma once

#include "base_types.h"

template <typename T>
struct bump_array
{
	T *data;
	usize count;
	usize capacity;

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

template <typename T>
inline T *
array_add(bump_array<T> *array, const IDENTITY(T) &value)
{
	Assert(array->count < array->capacity);

	T *result = &array->data[array->count];
	*result = value;
	array->count++;

	return result;
}

template <typename T>
inline bump_array<T>
push_bump_array(Arena *arena, usize capacity)
{
	bump_array<T> result = {};
	result.data = push_array<T>(arena, capacity);
	result.capacity = capacity;

	return result;
}
