#pragma once

#include "base_types.h"
#include "base_string.h"

#include <stdlib.h> // for malloc
#include <stdio.h> // for fopen

extern "C"
{
__declspec(dllimport) unsigned long __stdcall
GetModuleFileNameA(struct HINSTANCE__ *hModule,
				   char *lpFilename,
				   unsigned long nSize);

__declspec(dllimport) unsigned long __stdcall
GetFullPathNameA(const char *lpFileName,
				 unsigned long nBufferLength,
				 char *lpBuffer,
				 char **lpFilePart);
}

inline string
get_executable_filepath()
{
	char *buf = (char *)push_size(&g_tempMemory, 260);

	u32 size = GetModuleFileNameA(nullptr, buf, 260);
	Assert(size != 0);

	string result;
	result.data = buf;
	result.count = size;

	return result;
}

inline string
get_absolute_filepath(string relative_path)
{
	char *relative_path_cstr = to_cstring(relative_path);

	u32 absolute_path_size = GetFullPathNameA(relative_path_cstr, 0, nullptr, nullptr);
	if (absolute_path_size == 0)
	{
		return {};
	}

	char *absolute_path = (char *)push_size(&g_tempMemory, absolute_path_size);

	u32 written = GetFullPathNameA(relative_path_cstr, absolute_path_size, absolute_path, nullptr);
	if (!(written != 0 && written < absolute_path_size))
	{
		return {};
	}

	string result;
	result.data = absolute_path;
	result.count = absolute_path_size - 1;

	return result;
}

inline string
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

inline string
read_entire_file(string filepath)
{
	char *cstr = to_cstring(filepath);
	return read_entire_file(cstr);
}

inline string
get_executable_dir()
{
	string exe_file_path = get_executable_filepath();
	string exe_file_dir = strip_filename(exe_file_path);
	return exe_file_dir;
}
