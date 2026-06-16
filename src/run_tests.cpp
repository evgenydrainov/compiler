#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>

internal void
TestReturnCode(const char *testName, int expectedCode)
{
	{
		char command[1024];
		sprintf_s(command, "..\\build\\Debug\\compiler.exe %s.c", testName);

		int compilationResult = system(command);
		if (compilationResult != 0)
		{
			fprintf(stderr, "Test %s failed: compilation error\n", testName);
			exit(1);
		}
	}

	{
		char command[1024];
		sprintf_s(command, "%s.exe", testName);

		int code = system(command);
		if (code != expectedCode)
		{
			fprintf(stderr, "Test %s failed: result is %d, but expected %d\n", testName, code, expectedCode);
			exit(1);
		}

		printf("Test %s passed (result=%d)\n", testName, code);
	}
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
}
