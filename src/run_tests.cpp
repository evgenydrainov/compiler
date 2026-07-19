#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <direct.h>
#include <process.h>

global_variable char compilerPath[1024];

internal void
TestReturnCode(char *testName, int expectedCode)
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

internal void
TestCompileError(char *testName)
{
	char inputPath[1024];
	sprintf_s(inputPath, "%s.c", testName);

	if (_spawnl(_P_WAIT,
				compilerPath,
				"compiler",
				inputPath,
				nullptr) == 0)
	{
		fprintf(stderr, "Test %s failed: the program compiled, but it was not supposed to.\n", testName);
		exit(1);
	}

	printf("Test %s passed\n", testName);
}

int main()
{
	char *currentDir = _getcwd(nullptr, 0);
	if (!currentDir)
	{
		fprintf(stderr, "_getcwd failed\n");
		exit(1);
	}

	sprintf_s(compilerPath, "%s\\compiler.exe", currentDir);

	if (_chdir("tests") != 0)
	{
		fprintf(stderr, "_chdir failed\n");
		exit(1);
	}
	
	//TestReturnCode("01_arithmetic", 0);
	//TestReturnCode("02_variables_scope", 0);
	//TestReturnCode("03_control_flow", 0);
	//TestReturnCode("04_functions", 0);
	//TestReturnCode("05_pointers", 0);
	//TestReturnCode("06_structs", 0);
	//TestReturnCode("07_logical_comparison", 0);
	//TestReturnCode("08_bitwise", 0);
	//TestReturnCode("09_casts_and_unsigned", 0);
	//TestReturnCode("10_arrays", 0);
	//TestReturnCode("11_enums", 0);
	//TestReturnCode("12_foreign_function", 0);
	//TestReturnCode("13_coroutine", 0);
	//TestReturnCode("14_short_circuit", 0);
	//TestReturnCode("15_break_continue", 0);
	//TestReturnCode("16_nested_aggregates", 0);
	//TestReturnCode("17_array_of_structs", 0);
	//TestReturnCode("18_floats", 0);

	if (_chdir("should_fail") != 0)
	{
		fprintf(stderr, "_chdir failed\n");
		exit(1);
	}

	//TestCompileError("01_assign_to_literal");
	//TestCompileError("02_redeclaration");
	//TestCompileError("03_type_mismatch_return");
	//TestCompileError("04_undeclared_variable");
	//TestCompileError("05_unknown_field");
	//TestCompileError("06_wrong_arg_count");

	if (_chdir("..\\..\\examples\\01_raylib") != 0)
	{
		fprintf(stderr, "_chdir failed\n");
		exit(1);
	}

	//TestReturnCode("main", 0);

	if (_chdir("..\\02_breakout") != 0)
	{
		fprintf(stderr, "_chdir failed\n");
		exit(1);
	}

	TestReturnCode("main", 0);
}
