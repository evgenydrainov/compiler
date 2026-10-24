#pragma once

#include "base/base.h"

enum VCRuntimeType : u32
{
	VCRuntimeType_Static,
	VCRuntimeType_Dynamic,
};

struct CompileOptions
{
	string inputFilePath;
	string outputFilePathNoExt;

	string exeFileDir;

	dynamic_array<string> libraries;
	dynamic_array<string> systemLibraries;

	bool useVendorLld;
	bool printCode;
	bool verboseMode;

	VCRuntimeType vcruntimeType;
};

enum CompileResult : u32
{
	CompileResult_Success,
	CompileResult_CannotReadFile,
	CompileResult_CannotWriteFile,
	CompileResult_ParseError,
	CompileResult_SemanticError,
	CompileResult_AssemblerError,
	CompileResult_LinkerError,
};

CompileResult Compile(CompileOptions *options);
