#include "compiler.h"

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic_pass.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <io.h>
#include <process.h>

internal string
LoadFile(const char *fileName)
{
	string result = {};
	
	FILE *file;
	fopen_s(&file, fileName, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		usize fileSize = ftell(file);

		char *fileData = (char *)malloc(fileSize + 1);
		if (fileData)
		{
			fseek(file, 0, SEEK_SET);
			if (fread(fileData, 1, fileSize, file) == fileSize)
			{
				fileData[fileSize] = 0;
				result = {fileData, fileSize};
			}
		}
		
		fclose(file);
	}

	return result;
}

struct FindLinkerResult
{
	const char *linkExePath;
	const char *libraryPath;
};

internal FindLinkerResult
FindLinkerPath()
{
	const char *versionsToTry[] =
	{
		"14.44.35207",
		"14.42.34433",
	};

	for (const char *version : versionsToTry)
	{
		char linkExePath[1024];
		sprintf_s(linkExePath,
				  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\%s\\bin\\Hostx64\\x64\\link.exe",
				  version);

		char libraryPath[1024];
		sprintf_s(libraryPath,
				  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\%s\\lib\\x64",
				  version);

		if (_access(linkExePath, 0) == 0)
		{
			FindLinkerResult result = {};

			result.linkExePath = _strdup(linkExePath);
			result.libraryPath = _strdup(libraryPath);

			return result;
		}
	}

	fprintf(stderr, "could not find link.exe");
	exit(1);
}

CompileResult
Compile(CompileOptions options)
{
	string sourceCode = LoadFile(options.inputFilePath);
	if (sourceCode.count == 0)
	{
		fprintf(stderr, "cannot open file '%s' for reading\n", options.inputFilePath);
		return CompileResult_CannotReadFile;
	}

	defer { free(sourceCode.data); };

	Lexer lexer = {};
	lexer.line = 1;
	lexer.current = sourceCode.data;

	Arena arena = {};
	arena.capacity = Megabytes(1);
	arena.data = (u8 *)malloc(arena.capacity);

	defer { free(arena.data); };

	Parser parser = {};
	parser.current = GetToken(&lexer);

	Node *program = ParseProgram(&parser, &lexer, &arena);

	if (!program
		|| parser.hadError)
	{
		return CompileResult_ParseError;
	}

	SemanticContext semanticContext = {};
	SemanticPass(program, &semanticContext, &arena);

	if (semanticContext.hadError)
	{
		return CompileResult_SemanticError;
	}

	// PrintTree(program, "", false);

	CodegenContext codegenContext = {};

	{
		char fileName[1024];
		sprintf_s(fileName, "%s.asm", options.outputFilePath);

		FILE *out;
		fopen_s(&out, fileName, "wb");
		if (!out)
		{
			fprintf(stderr, "cannot open file '%s' for writing\n", fileName);
			return CompileResult_CannotWriteFile;
		}

		Generate_x86_64(program, out, &codegenContext);
		fclose(out);
	}

	{
		char outputPath[1024];
		sprintf_s(outputPath, "%s.obj", options.outputFilePath);

		char inputPath[1024];
		sprintf_s(inputPath, "%s.asm", options.outputFilePath);

		if (_spawnl(_P_WAIT,
					"C:\\Users\\Username\\AppData\\Local\\bin\\NASM\\nasm.exe",
					"nasm",
					"-f", "win64",
					"-o", outputPath,
					inputPath,
					nullptr) != 0)
		{
			return CompileResult_NasmError;
		}
	}

	{
		FindLinkerResult findResult = FindLinkerPath();

		char inputPath[1024];
		sprintf_s(inputPath, "%s.obj", options.outputFilePath);

		char libraryArg[1024];
		sprintf_s(libraryArg, "/LIBPATH:\"%s\"", findResult.libraryPath);

		if (_spawnl(_P_WAIT,
					findResult.linkExePath,
					"link",
					"/nologo",
					inputPath,
					"msvcrt.lib",
					"legacy_stdio_definitions.lib",
					libraryArg,
					"/LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\ucrt\\x64\"",
					"/LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\um\\x64\"",
					nullptr) != 0)
		{
			return CompileResult_LinkerError;
		}
	}

	// printf("Arena usage: %.2f%%\n", 100.0f*(arena.pos/(float)arena.capacity));

	return CompileResult_Success;
}
