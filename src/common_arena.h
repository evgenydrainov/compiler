#pragma once

#include "common_types.h"

constexpr usize DEFAULT_ALIGNMENT = 2 * sizeof(void *);

struct Arena
{
	u8 *data;
	usize pos;
	usize capacity;
};

extern Arena g_tempMemory;

inline bool
IsPowerOfTwo(usize x)
{
	bool result = ((x != 0)
				   && ((x & (x - 1)) == 0));
	return result;
}

inline usize
AlignForward(usize ptr, usize alignment)
{
	Assert(IsPowerOfTwo(alignment));
	return (ptr + (alignment-1)) & ~(alignment-1);
}

inline void *
PushSize(Arena *arena,
		 usize size,
		 usize alignment = DEFAULT_ALIGNMENT)
{
	usize alignedPos = AlignForward(arena->pos, alignment);

	Assert(alignedPos + size <= arena->capacity);

	void *result = arena->data + alignedPos;
	arena->pos = alignedPos + size;

	MemSet(result, 0, size);

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
PushStruct(Arena *arena)
{
	T *result = (T *)PushSize(arena, sizeof(T));
	return result;
}

template <typename T>
inline T *
PushArray(Arena *arena, usize count)
{
	T *result = (T *)PushSize(arena, count*sizeof(T));
	return result;
}

inline Arena
PushArena(Arena *arena, usize capacity)
{
	Arena result = {};
	result.data = (u8 *)PushSize(arena, capacity);
	result.capacity = capacity;

	return result;
}
