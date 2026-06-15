#include "common.h"
#include <stdlib.h>
#include <direct.h>
#include <windows.h>

int main()
{
	_chdir("test");

	system("..\\build\\Debug\\compiler.exe test.c");

	//STARTUPINFOA si = { sizeof(si) };
	//PROCESS_INFORMATION pi;

	//if (CreateProcessA(NULL, "..\\build\\Debug\\compiler.exe test.c", NULL, NULL, true, 0, NULL, NULL, &si, &pi))
	//{
	//	CloseHandle(pi.hProcess);
	//	CloseHandle(pi.hThread);
	//}
}
