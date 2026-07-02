#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <direct.h>
#include <process.h>

global_variable char compilerPath[1024];

internal void
TestReturnCode(const char *testName, int expectedCode)
{
	//clock_t start = clock();

	{
		char inputPath[1024];
		sprintf_s(inputPath, "%s.c", testName);

		if (_spawnl(_P_WAIT,
					compilerPath,
					"compiler",
					inputPath,
					nullptr) != 0)
		{
			fprintf(stderr, "Test %s failed: compilation error\n", testName);
			exit(1);
		}
	}

	{
		char exePath[1024];
		sprintf_s(exePath, "%s.exe", testName);

		int code = (int)_spawnl(_P_WAIT, exePath, testName, nullptr);
		if (code != expectedCode)
		{
			fprintf(stderr, "Test %s failed: result is %d, but expected %d\n", testName, code, expectedCode);
			exit(1);
		}

		printf("Test %s passed (result=%d)\n", testName, code);
	}

	//clock_t end = clock();
	//double elapsed_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
	//printf("Elapsed time: %.0f ms\n", elapsed_ms);
}

int main()
{
	char *currentDir = _getcwd(nullptr, 0);
	if (!currentDir)
	{
		fprintf(stderr, "_getcwd failed\n");
		exit(1);
	}

	sprintf_s(compilerPath, "%s\\build\\Debug\\compiler.exe", currentDir);

	if (_chdir("tests") != 0)
	{
		fprintf(stderr, "_chdir failed\n");
		exit(1);
	}
	
	//TestReturnCode("01_math_precedence",         14);
	//TestReturnCode("02_parenthesis_precedence",  20);
	//TestReturnCode("03_subtract",                7);
	//TestReturnCode("04_division",                14);
	//TestReturnCode("05_variables",               25);
	//TestReturnCode("06_variable_shadowing",      2);
	//TestReturnCode("07_while_loop",              10);
	//TestReturnCode("08_if_else",                 1);
	//TestReturnCode("09_function",                42);
	//TestReturnCode("10_function_argument_order", 7);
	//TestReturnCode("11_recursion",               120);
	//TestReturnCode("12_nested_function_calls",   10);
	//TestReturnCode("13_pointers",                20);
	//TestReturnCode("14_pointer_write",           40);
	//TestReturnCode("15_struct",                  0);
	//TestReturnCode("16_struct_stack_allocation", 7);
	//TestReturnCode("17_modulo",                  0);
	//TestReturnCode("18_for_loop",                10);
	//TestReturnCode("19_logical_and",             0);
	//TestReturnCode("20_logical_or",              0);
	//TestReturnCode("21_if_else",                 0);
	//TestReturnCode("22_foreign_function",        0);
	TestReturnCode("23_enum",                    0);

	if (_chdir("..\\examples\\01_raylib") != 0)
	{
		fprintf(stderr, "_chdir failed\n");
		exit(1);
	}

	//TestReturnCode("main", 0);
}
