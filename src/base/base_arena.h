#pragma once

#include "base_types.h"
#include "base_slice.h"
#include <string.h> // for memset
#include <stdlib.h> // for malloc

constexpr usize DEFAULT_ALIGNMENT = 2 * sizeof(void *);

struct Arena
{
	u8 *data;
	usize pos;
	usize capacity;
	u32 numTimesReallocated;
};

extern Arena g_tempArena;

inline void *
push_size(Arena *arena,
		  usize size,
		  usize alignment = DEFAULT_ALIGNMENT)
{
	Assert(size <= arena->capacity && "allocation too large");

	usize alignedPos = align_forward(arena->pos, alignment);

	if (alignedPos + size > arena->capacity)
	{
		arena->data = (u8 *)malloc(arena->capacity);
		arena->pos = 0;
		arena->numTimesReallocated++;
		alignedPos = 0;
	}

	void *result = arena->data + alignedPos;
	arena->pos = alignedPos + size;

	memset(result, 0, size);

#if 0
	f32 percentage = arena->pos/(f32)arena->capacity;
	printf("%f\n", percentage);
	if (percentage > 0.90)
	{
		__debugbreak();
	}
#endif

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

inline void *
grow_allocation(Arena *arena, void *ptr, usize oldSize, usize newSize)
{
	Assert(newSize >= oldSize);

	if (!ptr)
	{
		return push_size(arena, newSize);
	}

	// assert that this allocation was the last one for this arena
	Assert(arena->data + arena->pos == (u8 *)ptr + oldSize);

	usize growSize = newSize - oldSize;

	// assert that reallocation will not happen in push_size()
	Assert(arena->pos + growSize <= arena->capacity);
	push_size(arena, growSize, 1);

	return ptr;
}
