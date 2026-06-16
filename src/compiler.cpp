#include "compiler.h"

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic_pass.h"

#include <stdlib.h>
#include <stdio.h>

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
	arena.capacity = Megabytes(10);
	arena.data = (u8 *)malloc(arena.capacity);

	defer { free(arena.data); };

	Parser parser = {};
	parser.current = GetToken(&lexer);

	AstNode *program = ParseProgram(&parser, &lexer, &arena);

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
		char nasmCall[1024];
		sprintf_s(nasmCall, "%%USERPROFILE%%\\AppData\\Local\\bin\\NASM\\nasm.exe -f win64 %s.asm -o %s.obj", options.outputFilePath, options.outputFilePath);

		if (system(nasmCall) != 0)
		{
			return CompileResult_NasmError;
		}
	}

	{
		char linkerCall[1024];
		sprintf_s(linkerCall,
				  "\""
				  "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.44.35207\\bin\\Hostx64\\x64\\link.exe\""
				  " /nologo %s.obj msvcrt.lib legacy_stdio_definitions.lib"
				  " /LIBPATH:\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.44.35207\\lib\\x64\""
				  " /LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\ucrt\\x64\""
				  " /LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\um\\x64\""
				  "\"",
				  options.outputFilePath);

		if (system(linkerCall) != 0)
		{
			return CompileResult_LinkerError;
		}
	}

	// printf("Arena usage: %.2f%%\n", 100.0f*(arena.pos/(float)arena.capacity));

	return CompileResult_Success;
}
