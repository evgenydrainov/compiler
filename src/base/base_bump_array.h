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
inline void
array_add(bump_array<T> *array, const T &value)
{
	Assert(array->count < array->capacity);

	array->data[array->count] = value;
	array->count++;
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
