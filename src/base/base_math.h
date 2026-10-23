#pragma once

#include "base_types.h"
#include "base_assert.h"

template <typename T>
inline T
max(T a, T b)
{
	T result = (a >= b) ? a : b;
	return result;
}

template <typename T>
inline T
min(T a, T b)
{
	T result = (a < b) ? a : b;
	return result;
}

inline bool
is_power_of_two(usize x)
{
	bool result = (x != 0) && ((x & (x - 1)) == 0);
	return result;
}

inline usize
align_forward(usize ptr, usize alignment)
{
	Assert(is_power_of_two(alignment));
	return (ptr + (alignment-1)) & ~(alignment-1);
}

inline usize
align_downward(usize ptr, usize alignment)
{
	Assert(is_power_of_two(alignment));
	return ptr & ~(alignment-1);
}
