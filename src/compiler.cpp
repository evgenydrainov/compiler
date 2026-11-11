#include "compiler.h"

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic_pass.h"
#include "print_code.h"
#include "profiler.h"

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
	string nasmPath = string_format(temp_allocator(), "%svendor\\nasm\\nasm.exe", options->compilerInstallDir);

	char *cstr = to_cstring(nasmPath, temp_allocator());

	if (_access(cstr, 0) == 0)
	{
		return nasmPath;
	}

	fprintf(stderr, "could not find nasm.exe\n");
	exit(1);
}

internal string
FindLLDLink(CompileOptions *options)
{
	string lldPath = string_format(temp_allocator(), "%svendor\\LLVM\\lld-link.exe", options->compilerInstallDir);

	char *cstr = to_cstring(lldPath, temp_allocator());

	if (_access(cstr, 0) == 0)
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

	if (res.windows_sdk_version != 0)
	{
		return res;
	}

	fprintf(stderr, "could not find link.exe\n");
	exit(1);
}

internal int
CompilerRunProcess(CompileOptions *options, slice<string> commandLine)
{
	if (options->verboseMode)
	{
		file_print(stdout, "[VERBOSE] running command: ");
		for (string it : commandLine)
		{
			file_print(stdout, "%s ", it);
		}
		file_print(stdout, "\n");
	}

	return run_process(commandLine);
}

internal void
PrintSize(usize value)
{
	f64 valueF64 = (f64)value;
	if (valueF64 >= 1024)
	{
		valueF64 /= 1024;
		if (valueF64 >= 1024)
		{
			valueF64 /= 1024;
			printf("%.2f MB", valueF64);
		}
		else
		{
			printf("%.2f KB", valueF64);
		}
	}
	else
	{
		printf("%.2f B", valueF64);
	}
}

internal void
PrintArenaUsage(Arena *arena)
{
	f32 percentage = 100.0f*(arena->maxPos/(float)arena->capacity);
	printf("%.2f%% (", percentage);
	PrintSize(arena->maxPos);
	printf(" / ");
	PrintSize(arena->capacity);
	printf(") (num times reallocated: %u)\n", arena->numTimesReallocated);
}

CompileResult
Compile(CompileOptions *options)
{
	PROFILE_FUNCTION();

	options->compilerInstallDir = get_executable_dir(heap_allocator());

	LexerContext lexerContext = {};
	lexerContext.compilerInstallDir = options->compilerInstallDir;
	lexerContext.modulesDir = string_concat(options->compilerInstallDir, "modules/", heap_allocator());

	Lexer lexer = {};
	lexer.context = &lexerContext;

	{
		string builtinFilePath = string_concat(options->compilerInstallDir, "modules/builtin/builtin.c", heap_allocator());

		string builtinFileSrc = read_entire_file(builtinFilePath, heap_allocator());
		if (!builtinFileSrc)
		{
			file_print(stderr, "cannot open file '%s' for reading\n", builtinFilePath);
			return CompileResult_CannotReadFile;
		}

		lexer.line = 1;
		lexer.current = builtinFileSrc.data;
		lexer.fileName = builtinFilePath;
		lexer.lineStart = lexer.current;
	}

	{
		string sourceCode = read_entire_file(options->inputFilePath, heap_allocator());
		if (!sourceCode)
		{
			file_print(stderr, "cannot open file '%s' for reading\n", options->inputFilePath);
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
	arena.capacity = megabytes(4);
	arena.data = (u8 *)malloc(arena.capacity);

	Parser parser = {};
	parser.options = options;
	parser.current = GetToken(&lexer);

	Node *program = ParseProgram(&parser, &lexer, &arena);

	reset_temporary_storage();

	if (!program
		|| parser.hadError)
	{
		return CompileResult_ParseError;
	}

	SemanticContext semanticContext = {};
	semanticContext.arena = &arena;
	SemanticPass(program, &semanticContext, &arena);

	reset_temporary_storage();

	if (options->printCode)
	{
		PrintContext context = {};
		PrintProgram(&context, program);

		reset_temporary_storage();
	}

	if (semanticContext.hadError)
	{
		return CompileResult_SemanticError;
	}

	CodegenContext codegenContext = {};
	codegenContext.cstringLiterals = semanticContext.cstringLiterals;
	codegenContext.stringLiterals = semanticContext.stringLiterals;

	string asmFilePath = string_concat(options->outputFilePathNoExt, ".asm", heap_allocator());
	string objFilePath = string_concat(options->outputFilePathNoExt, ".obj", heap_allocator());

	{
		char *asmFilePathCStr = to_cstring(asmFilePath, temp_allocator());

		FILE *out;
		fopen_s(&out, asmFilePathCStr, "wb");

		if (!out)
		{
			fprintf(stderr, "cannot open file '%s' for writing\n", asmFilePathCStr);
			return CompileResult_CannotWriteFile;
		}

		codegenContext.out = out;
		Generate_x86_64(program, &codegenContext);

		reset_temporary_storage();

		fclose(out);
	}

	{
		PROFILE_SCOPE("nasm.exe");

		string nasmPath = FindNasm(options);

		dynamic_array<string> commandLine = {};
		commandLine.allocator = temp_allocator();

		array_add(&commandLine, nasmPath);

		if (options->generateDebugInfo)
		{
			array_add(&commandLine, "-g");
		}

		array_add(&commandLine, "-f");
		array_add(&commandLine, "win64");
		array_add(&commandLine, "-o");
		array_add(&commandLine, objFilePath);
		array_add(&commandLine, asmFilePath);

		if (CompilerRunProcess(options, commandLine) != 0)
		{
			return CompileResult_AssemblerError;
		}
	}

	if (options->useVendorLld)
	{
		PROFILE_SCOPE("lld-link.exe");

		string lldExePath = FindLLDLink(options);

		dynamic_array<string> commandLine = {};
		commandLine.allocator = temp_allocator();

		array_add(&commandLine, lldExePath);

		if (options->generateDebugInfo)
		{
			array_add(&commandLine, "/DEBUG");
		}

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
		PROFILE_SCOPE("link.exe");

		Find_Result res = FindLinker();

		string linkExePath    = string_format(temp_allocator(), "\"%s\\link.exe\"", res.vs_exe_path_a);
		string vsLibpathArg   = string_format(temp_allocator(), "/LIBPATH:\"%s\"",  res.vs_library_path_a);
		string ucrtLibpathArg = string_format(temp_allocator(), "/LIBPATH:\"%s\"",  res.windows_sdk_ucrt_library_path_a);
		string umLibpathArg   = string_format(temp_allocator(), "/LIBPATH:\"%s\"",  res.windows_sdk_um_library_path_a);

		dynamic_array<string> commandLine = {};
		commandLine.allocator = temp_allocator();

		array_add(&commandLine, linkExePath);
		array_add(&commandLine, "/NOLOGO");

		if (options->generateDebugInfo)
		{
			array_add(&commandLine, "/DEBUG");
		}

		array_add(&commandLine, "/INCREMENTAL:NO");
		array_add(&commandLine, "/OPT:REF");
		array_add(&commandLine, "/OPT:ICF");
		array_add(&commandLine, "/SUBSYSTEM:CONSOLE");
		array_add(&commandLine, objFilePath);

		for (string lib : options->libraries)
		{
			array_add(&commandLine, lib);
		}

		for (string lib : options->systemLibraries)
		{
			array_add(&commandLine, lib);
		}

		if (options->vcruntimeType == VCRuntimeType_Static)
		{
			array_add(&commandLine, "libcmt.lib");
		}
		else if (options->vcruntimeType == VCRuntimeType_Dynamic)
		{
			array_add(&commandLine, "msvcrt.lib");
			array_add(&commandLine, "legacy_stdio_definitions.lib");
		}

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
		printf("[VERBOSE] arena usage: ");
		PrintArenaUsage(&arena);
		printf("[VERBOSE] temp arena usage: ");
		PrintArenaUsage(&g_tempArena);
	}

	reset_temporary_storage();

	return CompileResult_Success;
}
