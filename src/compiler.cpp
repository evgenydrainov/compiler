#include "compiler.h"

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic_pass.h"
#include "print_code.h"

#pragma warning(push, 0)
#define MICROSOFT_CRAZINESS_IMPLEMENTATION
#include "microsoft_craziness.h"
#pragma warning(pop)

#include "subprocess.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <io.h>
#include <process.h>

internal char *
FindNasm(CompileOptions *options)
{
	string nasmPath = tprintf(STR_FMT "\\vendor\\nasm\\nasm.exe", STR_ARG(options->exeFileDir));

	if (_access(nasmPath.data, 0) == 0)
	{
		return nasmPath.data;
	}

#if 0
	usize dummy;

	char appdata[512];
	getenv_s(&dummy, appdata, "LOCALAPPDATA");

	char nasmPath[512];
	sprintf_s(nasmPath, "%s\\bin\\NASM\\nasm.exe", appdata);

	if (_access(nasmPath, 0) == 0)
	{
		return _strdup(nasmPath);
	}
#endif

	fprintf(stderr, "could not find nasm.exe\n");
	exit(1);
}

internal char *
FindLLDLink(CompileOptions *options)
{
	string lldPath = tprintf(STR_FMT "\\vendor\\LLVM\\lld-link.exe", STR_ARG(options->exeFileDir));

	if (_access(lldPath.data, 0) == 0)
	{
		return lldPath.data;
	}

	fprintf(stderr, "could not find lld-link.exe\n");
	exit(1);
}

internal Find_Result
FindLinker()
{
	Find_Result res = find_visual_studio_and_windows_sdk();

	if (res.windows_sdk_version == 0)
	{
		fprintf(stderr, "could not find link.exe\n");
	}

	return res;
}

internal int
process_create(slice<char *> commandLine, subprocess_s *out_process)
{
	*out_process = {};

	dynamic_array<char> commandLineCombined = {};

	for (char *arg : commandLine)
	{
		for (char *it = arg; *it; it++)
		{
			array_add(&commandLineCombined, *it);
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

internal int
run_process(slice<char *> commandLine)
{
	subprocess_s process;
	if (process_create(commandLine, &process) != 0)
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

CompileResult
Compile(CompileOptions *options)
{
	options->exeFileDir = get_executable_dir();
	string modulesDir = string_concat(options->exeFileDir, "modules/");

	LexerContext lexerContext = {};
	lexerContext.compilerExeFileDir = options->exeFileDir;
	lexerContext.modulesDir = modulesDir;

	Lexer lexer = {};
	lexer.context = &lexerContext;

	{
		string builtinFilePath = string_concat(options->exeFileDir, "modules/builtin.c");

		string builtinFileSrc = read_entire_file(builtinFilePath);
		if (builtinFileSrc.count == 0)
		{
			fprintf(stderr, "cannot open file " STR_FMT_QUOTED " for reading\n", STR_ARG(builtinFilePath));
			return CompileResult_CannotReadFile;
		}

		lexer.line = 1;
		lexer.current = builtinFileSrc.data;
		lexer.fileName = builtinFilePath;
		lexer.lineStart = lexer.current;
	}

	{
		string sourceCode = read_entire_file(options->inputFilePath);
		if (sourceCode.count == 0)
		{
			fprintf(stderr, "cannot open file " STR_FMT_QUOTED " for reading\n", STR_ARG(options->inputFilePath));
			return CompileResult_CannotReadFile;
		}

		LexerFrame frame = {};
		frame.current = sourceCode.data;
		frame.line = 1;
		frame.fileName = options->inputFilePath;
		frame.lineStart = frame.current;

		array_add(&lexer.includeStack, frame);
	}

	Arena arena = {};
	arena.capacity = Megabytes(4);
	arena.data = (u8 *)malloc(arena.capacity);

	defer { free(arena.data); };

	Parser parser = {};
	parser.options = options;
	parser.current = GetToken(&lexer);

	Node *program = ParseProgram(&parser, &lexer, &arena);

	if (!program
		|| parser.hadError)
	{
		return CompileResult_ParseError;
	}

	SemanticContext semanticContext = {};
	semanticContext.arenaForAst = &arena;
	SemanticPass(program, &semanticContext, &arena);

	if (semanticContext.hadError)
	{
		return CompileResult_SemanticError;
	}

	{
		//PrintContext context = {};
		//PrintProgram(&context, program);
	}

	CodegenContext codegenContext = {};
	codegenContext.cstringLiterals = semanticContext.cstringLiterals;
	codegenContext.stringLiterals = semanticContext.stringLiterals;
	codegenContext.deferStack = push_bump_array<Node *>(&arena, 256);

	string asmFilePath = string_concat(options->outputFilePath, ".asm");
	string objFilePath = string_concat(options->outputFilePath, ".obj");

	char *asmFilePathCStr = to_cstring(asmFilePath);
	char *objFilePathCStr = to_cstring(objFilePath);

	{
		FILE *out;
		fopen_s(&out, asmFilePathCStr, "wb");

		if (!out)
		{
			fprintf(stderr, "cannot open file '%s' for writing\n", asmFilePathCStr);
			return CompileResult_CannotWriteFile;
		}

		codegenContext.out = out;
		Generate_x86_64(program, &codegenContext);

		fclose(out);
	}

	{
		char *nasmPath = FindNasm(options);

		if (_spawnl(_P_WAIT,
					nasmPath,
					"nasm",
					"-g",
					"-f", "win64",
					"-o", objFilePathCStr,
					asmFilePathCStr,
					nullptr) != 0)
		{
			return CompileResult_NasmError;
		}
	}

	if (options->useVendorLld)
	{
		char *lldExePath = FindLLDLink(options);

		dynamic_array<char *> commandLine = {};

		array_add(&commandLine, lldExePath);
		array_add(&commandLine, "/DEBUG");
		array_add(&commandLine, "/OPT:REF");
		array_add(&commandLine, "/OPT:ICF");
		array_add(&commandLine, "/SUBSYSTEM:CONSOLE");
		array_add(&commandLine, objFilePathCStr);

		for (string lib : options->libraries)
		{
			array_add(&commandLine, to_cstring(lib));
		}

		int retcode = run_process(commandLine);
		if (retcode != 0)
		{
			return CompileResult_LinkerError;
		}
	}
	else
	{
		Find_Result res = FindLinker();

		char linkExePath[512];
		sprintf_s(linkExePath, "\"%s\\link.exe\"", res.vs_exe_path_a);

		char vsLibpathArg[512];
		sprintf_s(vsLibpathArg, "/LIBPATH:\"%s\"", res.vs_library_path_a);

		char ucrtLibpathArg[512];
		sprintf_s(ucrtLibpathArg, "/LIBPATH:\"%s\"", res.windows_sdk_ucrt_library_path_a);

		char umLibpathArg[512];
		sprintf_s(umLibpathArg, "/LIBPATH:\"%s\"", res.windows_sdk_um_library_path_a);

		dynamic_array<char *> commandLine = {};

		array_add(&commandLine, linkExePath);
		array_add(&commandLine, "/nologo");
		array_add(&commandLine, "/DEBUG");
		array_add(&commandLine, "/INCREMENTAL:NO");
		array_add(&commandLine, "/OPT:REF");
		array_add(&commandLine, "/OPT:ICF");
		array_add(&commandLine, "/SUBSYSTEM:CONSOLE");
		array_add(&commandLine, objFilePathCStr);

		for (string lib : options->libraries)
		{
			array_add(&commandLine, to_cstring(lib));
		}

		array_add(&commandLine, "libcmt.lib");
		array_add(&commandLine, "legacy_stdio_definitions.lib");
		array_add(&commandLine, "kernel32.lib");
		array_add(&commandLine, "user32.lib");
		array_add(&commandLine, "gdi32.lib");
		array_add(&commandLine, "shell32.lib");
		array_add(&commandLine, "winmm.lib");

		array_add(&commandLine, vsLibpathArg);
		array_add(&commandLine, ucrtLibpathArg);
		array_add(&commandLine, umLibpathArg);

		int retcode = run_process(commandLine);
		if (retcode != 0)
		{
			return CompileResult_LinkerError;
		}
	}

	// printf("Arena usage: %.2f%%\n", 100.0f*(arena.pos/(float)arena.capacity));

	return CompileResult_Success;
}
