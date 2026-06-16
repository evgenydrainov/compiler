#pragma once

#include "common.h"

struct CompileOptions
{
	const char *inputFilePath;
	const char *outputFilePath;
};

enum CompileResult : u32
{
	CompileResult_Success,
	CompileResult_CannotReadFile,
	CompileResult_CannotWriteFile,
	CompileResult_ParseError,
	CompileResult_SemanticError,
	CompileResult_NasmError,
	CompileResult_LinkerError,
};

CompileResult Compile(CompileOptions options);
