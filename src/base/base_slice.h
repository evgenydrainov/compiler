#pragma once

#include "base_types.h"

template <typename T>
struct dynamic_array;

template <typename T>
struct slice
{
	T *data;
	usize count;

	slice() = default;

	slice(dynamic_array<T> array);

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
