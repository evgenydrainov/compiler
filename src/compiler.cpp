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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <io.h>
#include <process.h>

internal string
FindNasm(CompileOptions *options)
{
	string nasmPath = tprintf(STR_FMT "vendor\\nasm\\nasm.exe", STR_ARG(options->exeFileDir));

	if (_access(nasmPath.data, 0) == 0)
	{
		return nasmPath;
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

internal string
FindLLDLink(CompileOptions *options)
{
	string lldPath = tprintf(STR_FMT "vendor\\LLVM\\lld-link.exe", STR_ARG(options->exeFileDir));

	if (_access(lldPath.data, 0) == 0)
	{
		return lldPath;
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
CompilerRunProcess(CompileOptions *options, slice<string> commandLine)
{
	if (options->verboseMode)
	{
		printf("[VERBOSE] running command: ");
		for (string it : commandLine)
		{
			printf(STR_FMT " ", STR_ARG(it));
		}
		printf("\n");
	}

	return run_process(commandLine);
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

	if (options->printCode)
	{
		PrintContext context = {};
		PrintProgram(&context, program);
	}

	CodegenContext codegenContext = {};
	codegenContext.cstringLiterals = semanticContext.cstringLiterals;
	codegenContext.stringLiterals = semanticContext.stringLiterals;
	codegenContext.deferStack = push_bump_array<Node *>(&arena, 256);

	string asmFilePath = string_concat(options->outputFilePathNoExt, ".asm");
	string objFilePath = string_concat(options->outputFilePathNoExt, ".obj");

	char *asmFilePathCStr = to_cstring(asmFilePath);

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
		string nasmPath = FindNasm(options);

		dynamic_array<string> commandLine = {};
		defer { array_free(&commandLine); };

		array_add(&commandLine, nasmPath);
		array_add(&commandLine, "-g");
		array_add(&commandLine, "-f");
		array_add(&commandLine, "win64");
		array_add(&commandLine, "-o");
		array_add(&commandLine, objFilePath);
		array_add(&commandLine, asmFilePath);

		if (CompilerRunProcess(options, commandLine) != 0)
		{
			return CompileResult_NasmError;
		}
	}

	if (options->useVendorLld)
	{
		string lldExePath = FindLLDLink(options);

		dynamic_array<string> commandLine = {};
		defer { array_free(&commandLine); };

		array_add(&commandLine, lldExePath);
		array_add(&commandLine, "/DEBUG");
		array_add(&commandLine, "/OPT:REF");
		array_add(&commandLine, "/OPT:ICF");
		array_add(&commandLine, "/SUBSYSTEM:CONSOLE");
		array_add(&commandLine, objFilePath);

		for (string lib : options->libraries)
		{
			array_add(&commandLine, lib);
		}

		int retcode = CompilerRunProcess(options, commandLine);
		if (retcode != 0)
		{
			return CompileResult_LinkerError;
		}
	}
	else
	{
		Find_Result res = FindLinker();

		string linkExePath = tprintf("\"%s\\link.exe\"", res.vs_exe_path_a);
		string vsLibpathArg = tprintf("/LIBPATH:\"%s\"", res.vs_library_path_a);
		string ucrtLibpathArg = tprintf("/LIBPATH:\"%s\"", res.windows_sdk_ucrt_library_path_a);
		string umLibpathArg = tprintf("/LIBPATH:\"%s\"", res.windows_sdk_um_library_path_a);

		dynamic_array<string> commandLine = {};
		defer { array_free(&commandLine); };

		array_add(&commandLine, linkExePath);
		array_add(&commandLine, "/nologo");
		array_add(&commandLine, "/DEBUG");
		array_add(&commandLine, "/INCREMENTAL:NO");
		array_add(&commandLine, "/OPT:REF");
		array_add(&commandLine, "/OPT:ICF");
		array_add(&commandLine, "/SUBSYSTEM:CONSOLE");
		array_add(&commandLine, objFilePath);

		for (string lib : options->libraries)
		{
			array_add(&commandLine, lib);
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

		int retcode = CompilerRunProcess(options, commandLine);
		if (retcode != 0)
		{
			return CompileResult_LinkerError;
		}
	}

	if (options->verboseMode)
	{
		printf("[VERBOSE] arena usage: %.2f%%\n", 100.0f*(arena.pos/(float)arena.capacity));
		printf("[VERBOSE] temp arena usage: %.2f%%\n", 100.0f*(g_tempArena.pos/(float)g_tempArena.capacity));
	}

	return CompileResult_Success;
}
