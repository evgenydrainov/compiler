#pragma once

#include "base/base.h"

enum VCRuntimeType : u32
{
	VCRuntimeType_Static,
	VCRuntimeType_Dynamic,
};

enum Subsystem : u32
{
	Subsystem_Console,
	Subsystem_Windows,
};

struct CompileOptions
{
	string inputFilePath;
	string outputFilePathNoExt;

	string compilerInstallDir;

	dynamic_array<string> libraries;
	dynamic_array<string> systemLibraries;

	bool useVendorLld;
	bool printCode;
	bool verboseMode;
	bool generateDebugInfo;

	VCRuntimeType vcruntimeType;
	Subsystem subsystem;
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
