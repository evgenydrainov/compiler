#pragma once

#include "base_types.h"
#include "base_assert.h"
#include <stdlib.h> // for malloc, free

enum AllocatorMode : u32
{
	AllocatorMode_Allocate,
	AllocatorMode_Resize,
	AllocatorMode_Free,
};

typedef void * (*AllocatorProc)(AllocatorMode mode,
								usize size, usize alignment,
								void *old_memory, usize old_size,
								void *userdata);

struct Allocator
{
	AllocatorProc proc;
	void *userdata;
};

inline Allocator heap_allocator();

inline void *
mem_alloc(usize size, usize alignment,
		  Allocator allocator)
{
	if (!allocator.proc)
	{
		allocator = heap_allocator(); // default allocator
	}
	return allocator.proc(AllocatorMode_Allocate, size, alignment, nullptr, 0, allocator.userdata);
}

inline void *
mem_resize(void *old_memory, usize old_size, usize new_size,
		   Allocator allocator)
{
	if (!allocator.proc)
	{
		allocator = heap_allocator(); // default allocator
	}
	return allocator.proc(AllocatorMode_Resize, new_size, 0, old_memory, old_size, allocator.userdata);
}

inline void
mem_free(void *old_memory, usize old_size,
		 Allocator allocator)
{
	if (!allocator.proc)
	{
		allocator = heap_allocator(); // default allocator
	}
	allocator.proc(AllocatorMode_Free, 0, 0, old_memory, old_size, allocator.userdata);
}

inline void *
heap_allocator_proc(AllocatorMode mode,
					usize size, usize alignment,
					void *old_memory, usize old_size,
					void *userdata)
{
	switch (mode)
	{
	case AllocatorMode_Allocate:
		return malloc(size);

	case AllocatorMode_Resize:
		return realloc(old_memory, size);

	case AllocatorMode_Free:
		free(old_memory);
		return nullptr;
	}

	Assert(false);
	return nullptr;
}

NO_STEP_INTO inline Allocator
heap_allocator()
{
	return {heap_allocator_proc, nullptr};
}
