#pragma once

#include "base_types.h"
#include "../subprocess.h"

typedef struct _STARTUPINFOA *LPSTARTUPINFOA;

extern "C"
{
__declspec(dllimport) int __stdcall
CreateProcessA(const char *lpApplicationName,
			   char *lpCommandLine,
			   LPSECURITY_ATTRIBUTES lpProcessAttributes,
			   LPSECURITY_ATTRIBUTES lpThreadAttributes,
			   int bInheritHandles,
			   unsigned long dwCreationFlags,
			   void *lpEnvironment,
			   const char *lpCurrentDirectory,
			   LPSTARTUPINFOA lpStartupInfo,
			   LPPROCESS_INFORMATION lpProcessInformation);
}

inline int
create_process(slice<string> commandLine, subprocess_s *out_process)
{
	*out_process = {};

	dynamic_array<char> commandLineCombined = {};
	defer { array_free(&commandLineCombined); };

	for (string arg : commandLine)
	{
		bool needQuote = string_needs_quoting(arg);

		if (needQuote)
		{
			array_add(&commandLineCombined, '"');
		}

		array_add_many(&commandLineCombined, arg);

		if (needQuote)
		{
			array_add(&commandLineCombined, '"');
		}

		array_add(&commandLineCombined, ' ');
	}

	array_add(&commandLineCombined, 0);

	subprocess_startup_info_s startInfo = { sizeof(startInfo) };
	subprocess_subprocess_information_s processInfo;

	if (!CreateProcessA(nullptr,
						commandLineCombined.data, // command line
						nullptr,		// process security attributes
						nullptr,		// primary thread security attributes
						1,				// handles are inherited
						0,				// creation flagsted
						nullptr,		// used environment
						nullptr,		// use parent's current directory
						(LPSTARTUPINFOA)&startInfo,
						(LPPROCESS_INFORMATION)&processInfo))
	{
		return -1;
	}

	out_process->hProcess = processInfo.hProcess;

	// We don't need the handle of the primary thread in the called process.
	CloseHandle(processInfo.hThread);

	out_process->alive = 1;

	return 0;
}

inline int
run_process(slice<string> commandLine)
{
	subprocess_s process;
	if (create_process(commandLine, &process) != 0)
	{
		return -1;
	}

	int retcode;
	if (subprocess_join(&process, &retcode) != 0)
	{
		return -1;
	}

	return retcode;
}
