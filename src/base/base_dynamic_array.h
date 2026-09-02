#pragma once

#include "base_types.h"
#include "base_slice.h"
#include <stdlib.h> // for realloc

template <typename T>
struct dynamic_array
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
array_reserve(dynamic_array<T> *array, usize want_capacity)
{
	if (want_capacity <= array->capacity)
	{
		return;
	}

	usize new_capacity = array->capacity;
	if (new_capacity == 0)
	{
		new_capacity = 8;
	}

	while (new_capacity < want_capacity)
	{
		new_capacity *= 2;
	}

	array->data = (T *)realloc(array->data, new_capacity*sizeof(T));
	array->capacity = new_capacity;
}

template <typename T>
inline void
array_free(dynamic_array<T> *array)
{
	free(array->data);
	array->data = nullptr;
	array->count = 0;
	array->capacity = 0;
}

template <typename T>
inline void
array_add(dynamic_array<T> *array,
		  typename identity<T>::type const &value)
{
	array_reserve(array, array->count+1);
	array->data[array->count++] = value;
}

template <typename T>
inline void
array_add_many(dynamic_array<T> *array,
			   slice<T> values)
{
	array_reserve(array, array->count + values.count);
	for (const T &value : values)
	{
		array->data[array->count++] = value;
	}
}

template <typename T>
slice<T>::slice(dynamic_array<T> array)
	: data(array.data), count(array.count)
{
}
