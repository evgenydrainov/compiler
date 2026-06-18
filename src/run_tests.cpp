#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <direct.h>
#include <process.h>

internal void
TestReturnCode(const char *testName, int expectedCode)
{
	//clock_t start = clock();

	{
		char inputPath[1024];
		sprintf_s(inputPath, "%s.c", testName);

		if (_spawnl(_P_WAIT,
					"..\\build\\Debug\\compiler.exe",
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
	if (_chdir("tests") != 0)
	{
		fprintf(stderr, "_chdir failed\n");
		exit(1);
	}
	
	TestReturnCode("01_math_precedence",         14);
	TestReturnCode("02_parenthesis_precedence",  20);
	TestReturnCode("03_subtract",                7);
	TestReturnCode("04_division",                14);
	TestReturnCode("05_variables",               25);
	TestReturnCode("06_variable_shadowing",      2);
	TestReturnCode("07_while_loop",              10);
	TestReturnCode("08_if_else",                 1);
	TestReturnCode("09_function",                42);
	TestReturnCode("10_function_argument_order", 7);
	TestReturnCode("11_recursion",               120);
	TestReturnCode("12_nested_function_calls",   10);
	TestReturnCode("13_pointers",                20);
}
