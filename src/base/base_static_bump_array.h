#pragma once

#include "base_types.h"

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