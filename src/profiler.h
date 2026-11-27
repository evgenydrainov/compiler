#pragma once

#include "base/base_types.h"
#include "base/base_time.h"

#ifndef ENABLE_PROFILER
#  define ENABLE_PROFILER 1 // enabled by default
#endif

#if ENABLE_PROFILER

#define PROFILE_SCOPE(scopeName) ProfileScope CONCATENATE(_profileScope, __LINE__) (__FILE__, __LINE__, scopeName)
#define PROFILE_FUNCTION()       ProfileScope CONCATENATE(_profileScope, __LINE__) (__FILE__, __LINE__, __FUNCTION__)

struct TimeRecord
{
	char *fileName;
	char *functionName;
	u64 cycleCount;
	int line;
	int callDepth;
};

struct Profiler
{
	TimeRecord records[32];
	int numRecords;
	int callDepth;
};

extern Profiler g_profiler;

struct ProfileScope
{
	TimeRecord *record;

	ProfileScope(char *fileName, int line, char *functionName)
	{
		record = &g_profiler.records[g_profiler.numRecords++];
		record->fileName = fileName;
		record->line = line;
		record->functionName = functionName;
		record->cycleCount -= get_performance_counter();
		record->callDepth = g_profiler.callDepth++;
	}

	~ProfileScope()
	{
		record->cycleCount += get_performance_counter();
		g_profiler.callDepth--;
	}
};

#else

#define PROFILE_SCOPE(scopeName)
#define PROFILE_FUNCTION()

#endif // ENABLE_PROFILER
