#pragma once

#include "common_types.h"
#include "common_string.h"

#ifdef _WIN32

extern "C"
{
__declspec(dllimport) unsigned long __stdcall
GetModuleFileNameA(void *hModule,
				   char *lpFilename,
				   unsigned long nSize);

__declspec(dllimport) unsigned long __stdcall
GetFullPathNameA(char *lpFileName,
				 unsigned long nBufferLength,
				 char *lpBuffer,
				 char **lpFilePart);
}

inline string
get_executable_filepath()
{
	char *buf = (char *)PushSize(&g_tempMemory, 260);

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

	char *absolute_path = (char *)PushSize(&g_tempMemory, absolute_path_size);

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

#else

#error TODO

#endif

string read_entire_file(char *filepath);

string read_entire_file(string filepath);

inline string
get_executable_dir()
{
	string executableFilePath = get_executable_filepath();

	string executableFileDir = strip_filename(executableFilePath);

	return executableFileDir;
}
