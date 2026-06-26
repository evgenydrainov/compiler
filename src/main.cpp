#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

int main(int argc, char *argv[])
{
	//{
	//	f32 f = 4.0f;
	//	u32 v = *(u32 *)&f;
	//	printf("%08x\n", v);
	//}

	if (argc != 2)
	{
		fprintf(stderr, "Usage: compiler <filename>\n");
		return 1;
	}

	char outputFilePath[1024];
	strcpy_s(outputFilePath, argv[1]);

	{
		size_t i;

		bool found = false;

		for (i = strlen(outputFilePath);
			 i--;)
		{
			if (outputFilePath[i] == '.')
			{
				found = true;
				break;
			}
			else if (outputFilePath[i] == '/'
					 || outputFilePath[i] == '\\')
			{
				fprintf(stderr, "filename is invalid\n");
				exit(1);
			}
		}

		if (!found)
		{
			fprintf(stderr, "filename is invalid\n");
			exit(1);
		}

		/*outputFilePath[i++] = '.';
		outputFilePath[i++] = 'a';
		outputFilePath[i++] = 's';
		outputFilePath[i++] = 'm';*/
		outputFilePath[i++] = 0;
	}

	CompileOptions options = {};
	options.inputFilePath = argv[1];
	options.outputFilePath = outputFilePath;

	CompileResult result = Compile(options);

	return result;
}
