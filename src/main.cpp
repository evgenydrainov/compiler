#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

Arena g_tempMemory;

void AssertionHandler(char *file, int line, char *condition)
{
	fprintf(stderr, "%s:%d: assertion failed: %s\n", file, line, condition);

#ifndef NDEBUG
	__debugbreak();
#endif

	exit(1);
}

int main(int argc, char *argv[])
{
	g_tempMemory.capacity = Megabytes(1);
	g_tempMemory.data = (u8 *)malloc(g_tempMemory.capacity);

	CompileOptions options = {};

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (strcmp(argv[i], "-print-code") == 0)
			{
				options.printCode = true;
			}
			else
			{
				fprintf(stderr, "unknown option '%s'\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			string inputFilePath = { argv[i], strlen(argv[i]) };
			inputFilePath = get_absolute_filepath(inputFilePath);

			options.inputFilePath = inputFilePath;
		}
	}

	if (options.inputFilePath.count == 0)
	{
		fprintf(stderr, "no input files provided\n");
		exit(1);
	}

	if (options.outputFilePathNoExt.count == 0)
	{
		string outputFilePathNoExt = strip_extension(options.inputFilePath);

		options.outputFilePathNoExt = outputFilePathNoExt;
	}

	CompileResult result = Compile(&options);

	return result;
}
