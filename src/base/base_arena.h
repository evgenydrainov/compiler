#pragma once

#include "base_types.h"
#include "base_slice.h"
#include "base_assert.h"
#include "base_math.h"
#include "base_allocator.h"
#include <string.h> // for memset
#include <stdlib.h> // for malloc

#ifndef CLEAR_TEMP_MEMORY_WHEN_RESETTING
#  ifdef NDEBUG
#    define CLEAR_TEMP_MEMORY_WHEN_RESETTING 0
#  else
#    define CLEAR_TEMP_MEMORY_WHEN_RESETTING 1
#  endif
#endif

struct Arena
{
	u8 *data;
	usize pos;
	usize capacity;
	usize maxPos;
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

	arena->maxPos = max(arena->maxPos, arena->pos);

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

inline bool
is_last_allocation(Arena *arena, void *ptr, usize size)
{
	return (arena->data + arena->pos == (u8 *)ptr + size);
}

inline void *
resize_allocation(Arena *arena, void *ptr, usize old_size, usize new_size)
{
	if (!ptr)
	{
		return push_size(arena, new_size);
	}

	if (new_size >= old_size)
	{
		// grow

		if (is_last_allocation(arena, ptr, old_size))
		{
			// fast path

			usize grow_size = new_size - old_size;

			// assert that reallocation will not happen in push_size()
			Assert(arena->pos + grow_size <= arena->capacity);
			push_size(arena, grow_size, 1);

			return ptr;
		}
		else
		{
			// slow path - we have to make a new allocation and copy the data

			void *new_ptr = push_size(arena, new_size);
			memcpy(new_ptr, ptr, old_size);
			return new_ptr;
		}
	}
	else
	{
		// shrink

		if (is_last_allocation(arena, ptr, old_size))
		{
			usize shrink_size = old_size - new_size;

			arena->pos -= shrink_size;

			return ptr;
		}
		else
		{
			// can't shrink - do nothing
			return ptr;
		}
	}
}

inline void *
arena_allocator_proc(AllocatorMode mode,
					 usize size, usize alignment,
					 void *old_memory, usize old_size,
					 void *userdata)
{
	Arena *arena = (Arena *)userdata;

	switch (mode)
	{
	case AllocatorMode_Allocate:
		return push_size(arena, size, alignment);

	case AllocatorMode_Resize:
		return resize_allocation(arena, old_memory, old_size, size);

	case AllocatorMode_Free:
		resize_allocation(arena, old_memory, old_size, 0);
		return nullptr;
	}

	Assert(false);
	return nullptr;
}

NO_STEP_INTO inline Allocator
arena_allocator(Arena *arena)
{
	return {arena_allocator_proc, arena};
}

NO_STEP_INTO inline Allocator
temp_allocator()
{
	return {arena_allocator_proc, &g_tempArena};
}

inline void
reset_temporary_storage()
{
#if CLEAR_TEMP_MEMORY_WHEN_RESETTING
	memset(g_tempArena.data, 0xff, g_tempArena.capacity);
#endif
	g_tempArena.pos = 0;
}
