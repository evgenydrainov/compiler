#pragma once

#include "base_types.h"
#include "base_slice.h"
#include <string.h> // for memset

constexpr usize DEFAULT_ALIGNMENT = 2 * sizeof(void *);

struct Arena
{
	u8 *data;
	usize pos;
	usize capacity;
};

extern Arena g_tempMemory;

inline void *
push_size(Arena *arena,
		  usize size,
		  usize alignment = DEFAULT_ALIGNMENT)
{
	usize alignedPos = align_forward(arena->pos, alignment);

	Assert(alignedPos + size <= arena->capacity);

	void *result = arena->data + alignedPos;
	arena->pos = alignedPos + size;

	memset(result, 0, size);

	/*
	f32 percentage = arena->pos/(f32)arena->capacity;
	printf("%f\n", percentage);
	if (percentage > 0.90)
	{
		int k = 123;
	}
	*/

	return result;
}

template <typename T>
inline T *
push_struct(Arena *arena)
{
	T *result = (T *)push_size(arena, sizeof(T));
	return result;
}

template <typename T>
inline T *
push_array(Arena *arena, usize count)
{
	T *result = (T *)push_size(arena, count*sizeof(T));
	return result;
}

inline Arena
push_arena(Arena *arena, usize capacity)
{
	Arena result = {};
	result.data = (u8 *)push_size(arena, capacity);
	result.capacity = capacity;

	return result;
}

template <typename T>
inline slice<T>
push_slice(Arena *arena, usize count)
{
	slice<T> result = {};
	result.data = (T *)push_size(arena, count*sizeof(T));
	result.count = count;

	return result;
}
