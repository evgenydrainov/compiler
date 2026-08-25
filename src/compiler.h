#pragma once

#include "base/base.h"

struct CompileOptions
{
	string inputFilePath;
	string outputFilePath;
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

CompileResult Compile(CompileOptions *options);
