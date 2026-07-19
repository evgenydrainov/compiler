#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

Arena g_tempMemory;

void AssertionHandler(char *file, int line, char *condition)
{
	fprintf(stderr, "%s:%d: assertion failed: %s\n", file, line, condition);
	__debugbreak();
}

int main(int argc, char *argv[])
{
	g_tempMemory.capacity = Megabytes(1);
	g_tempMemory.data = (u8 *)malloc(g_tempMemory.capacity);

	if (argc != 2)
	{
		fprintf(stderr, "Usage: compiler <filename>\n");
		return 1;
	}

	string inputFilePath = { argv[1], strlen(argv[1]) };
	inputFilePath = get_absolute_filepath(inputFilePath);

	string outputFilePath = strip_extension(inputFilePath);

	CompileOptions options = {};
	options.inputFilePath = inputFilePath;
	options.outputFilePath = outputFilePath;

	CompileResult result = Compile(&options);

	return result;
}
