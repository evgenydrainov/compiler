#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "profiler.h"

Arena g_tempArena;

void AssertionHandler(char *file, int line, char *condition, char *message)
{
	fprintf(stderr, "%s:%d: assertion failed: %s\n", file, line, (message) ? message : condition);
#ifndef NDEBUG
	__debugbreak();
#endif
	exit(1);
}

int main(int argc, char *argv[])
{
	g_tempArena.capacity = megabytes(1);
	g_tempArena.data = (u8 *)malloc(g_tempArena.capacity);

	CompileOptions options = {};
	options.generateDebugInfo = true; // enable by default

	bool readOutputFilePath = false;

	for (int i = 1; i < argc; i++)
	{
		string arg = {argv[i], strlen(argv[i])};

		if (readOutputFilePath)
		{
			string outputFilePath = get_absolute_filepath(arg, heap_allocator());
			options.outputFilePathNoExt = strip_extension(outputFilePath);

			readOutputFilePath = false;
			continue;
		}

		if (arg[0] == '-')
		{
			if (arg == "-print-code" || arg == "--print-code")
			{
				options.printCode = true;
			}
			else if (arg == "-verbose" || arg == "--verbose")
			{
				options.verboseMode = true;
			}
			else if (arg == "-o")
			{
				readOutputFilePath = true;
			}
			else if (arg == "-static-vcruntime" || arg == "--static-vcruntime")
			{
				options.vcruntimeType = VCRuntimeType_Static;
			}
			else if (arg == "-dynamic-vcruntime" || arg == "--dynamic-vcruntime")
			{
				options.vcruntimeType = VCRuntimeType_Dynamic;
			}
			else if (arg == "-debug" || arg == "--debug")
			{
				options.generateDebugInfo = true;
			}
			else if (arg == "-no-debug" || arg == "--no-debug")
			{
				options.generateDebugInfo = false;
			}
			else if (arg == "-time" || arg == "--time")
			{
				options.showProfileTimings = true;
			}
			else
			{
				fprintf(stderr, "unknown option '%s'\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			options.inputFilePath = get_absolute_filepath(arg, heap_allocator());
		}
	}

	if (readOutputFilePath)
	{
		fprintf(stderr, "missing filename after '-o'\n");
		exit(1);
	}

	if (!options.inputFilePath)
	{
		fprintf(stderr, "no input files provided\n");
		exit(1);
	}

	if (!options.outputFilePathNoExt)
	{
		options.outputFilePathNoExt = strip_extension(options.inputFilePath);
	}

	CompileResult result = Compile(&options);

#if ENABLE_PROFILER
	Assert(g_profiler.numRecords <= array_length(g_profiler.records));

	if (options.showProfileTimings)
	{
		printf("[PERF] profiler time records:\n");

		for (int i = 0; i < g_profiler.numRecords; i++)
		{
			TimeRecord *it = &g_profiler.records[i];
			f64 milliseconds = it->cycleCount * 1000.0 / (f64)get_performance_frequency();
			printf("[PERF] ");
			for (int j = 0; j < it->callDepth+1; j++)
			{
				printf("  ");
			}
			printf("%s: %.2fms\n", it->functionName, milliseconds);
		}
	}
#endif

	return result;
}
