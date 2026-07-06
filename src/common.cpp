#include "common.h"

#include <stdlib.h>
#include <stdio.h>

string
LoadFile(const char *fileName)
{
	string result = {};
	
	FILE *file;
	fopen_s(&file, fileName, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		usize fileSize = ftell(file);

		char *fileData = (char *)malloc(fileSize + 1);
		if (fileData)
		{
			fseek(file, 0, SEEK_SET);
			if (fread(fileData, 1, fileSize, file) == fileSize)
			{
				fileData[fileSize] = 0;
				result = {fileData, fileSize};
			}
		}
		
		fclose(file);
	}

	return result;
}

string
LoadFile(string fileName)
{
	char buf[1024];
	sprintf_s(buf, STR_FMT, STR_ARG(fileName));

	return LoadFile(buf);
}
