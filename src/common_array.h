#pragma once

#include "common_types.h"

/*
 *  BumpArray
 */

template <typename T>
struct BumpArray
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
array_add(BumpArray<T> *array, const T &value)
{
	Assert(array->count < array->capacity);

	array->data[array->count] = value;
	array->count++;
}

template <typename T>
inline BumpArray<T>
PushBumpArray(Arena *arena, usize capacity)
{
	BumpArray<T> result = {};
	result.data = PushArray<T>(arena, capacity);
	result.capacity = capacity;

	return result;
}

/*
 *  StaticBumpArray
 */

template <typename T, usize N>
struct StaticBumpArray
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
array_add(StaticBumpArray<T, N> *array, const T &value)
{
	Assert(array->count < array->capacity);

	T *result = &array->data[array->count];

	*result = value;
	array->count++;

	return result;
}
