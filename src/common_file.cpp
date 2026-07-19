#include "common.h"

#include <stdlib.h>
#include <stdio.h>

string
read_entire_file(char *filepath)
{
	string result = {};
	
	FILE *file;
	fopen_s(&file, filepath, "rb");
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
read_entire_file(string filepath)
{
	char *cstr = to_cstring(filepath);

	return read_entire_file(cstr);
}
