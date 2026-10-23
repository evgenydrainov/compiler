#pragma once

#include "base_types.h"

typedef union _LARGE_INTEGER LARGE_INTEGER;

extern "C"
{
__declspec(dllimport) int __stdcall
QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);

__declspec(dllimport) int __stdcall
QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
}

inline u64
get_performance_counter()
{
	u64 result;
	QueryPerformanceCounter((LARGE_INTEGER *)&result);
	return result;
}

inline u64
get_performance_frequency()
{
	u64 result;
	QueryPerformanceFrequency((LARGE_INTEGER *)&result);
	return result;
}
