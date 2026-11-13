#pragma once

#include "base_types.h"
#include "base_string.h"
#include "base_allocator.h"
#include "base_format.h"
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
get_executable_filepath(Allocator allocator)
{
	char *buffer = (char *)mem_alloc(260, DEFAULT_ALIGNMENT, allocator);

	u32 size = GetModuleFileNameA(nullptr, buffer, 260);
	Assert(size != 0);

	return {buffer, size};
}

inline string
get_absolute_filepath(string relative_path, Allocator allocator)
{
	char *relative_path_cstr = to_cstring(relative_path, temp_allocator());

	u32 absolute_path_size = GetFullPathNameA(relative_path_cstr, 0, nullptr, nullptr);
	if (absolute_path_size == 0)
	{
		return {};
	}

	char *absolute_path = (char *)mem_alloc(absolute_path_size, DEFAULT_ALIGNMENT, allocator);

	u32 written = GetFullPathNameA(relative_path_cstr, absolute_path_size, absolute_path, nullptr);
	if (!(written != 0 && written < absolute_path_size))
	{
		return {};
	}

	string result = {};
	result.data = absolute_path;
	result.count = absolute_path_size - 1;

	return result;
}

inline string
read_entire_file(char *filepath, Allocator allocator)
{
	string result = {};
	
	FILE *file;
	fopen_s(&file, filepath, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		usize filesize = ftell(file);

		char *filedata = (char *)mem_alloc(filesize + 1, DEFAULT_ALIGNMENT, allocator);
		if (filedata)
		{
			fseek(file, 0, SEEK_SET);
			if (fread(filedata, 1, filesize, file) == filesize)
			{
				filedata[filesize] = 0;
				result = {filedata, filesize};
			}
		}
		
		fclose(file);
	}

	return result;
}

inline string
read_entire_file(string filepath, Allocator allocator)
{
	char *cstr = to_cstring(filepath, temp_allocator());

	return read_entire_file(cstr, allocator);
}

inline string
get_executable_dir(Allocator allocator)
{
	string exe_file_path = get_executable_filepath(allocator);
	string exe_file_dir = strip_filename(exe_file_path);

	return exe_file_dir;
}

template <typename... Args>
inline void
file_print(FILE *file, string format, Args... args)
{
	Allocator allocator = temp_allocator();

	string_builder builder = {};
	builder_init(&builder, allocator);

	FormatContext context = {};
	context.format = format;
	context.builder = &builder;

	string_format_impl(&context, args...);

	string str = builder_to_string(*context.builder);
	fwrite(str.data, 1, str.count, file);

	array_free(&builder.buffer);
}
