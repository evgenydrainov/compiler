#pragma once

#include "base_types.h"

template <typename T>
struct dynamic_array;

struct string;

template <typename T>
struct slice
{
	T *data;
	usize count;

	slice() = default;

	NO_STEP_INTO slice(dynamic_array<T> array);

	NO_STEP_INTO slice(string str);

	template <usize N>
	NO_STEP_INTO slice(T (&array)[N]) : data(array), count(N) {}

	NO_STEP_INTO T &operator[](usize index)
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	NO_STEP_INTO const T &operator[](usize index) const
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	NO_STEP_INTO T *begin() { return &data[0]; }
	NO_STEP_INTO T *end()   { return &data[count]; }
};
