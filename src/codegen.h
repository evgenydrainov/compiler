#pragma once

#include "common.h"
#include "parser.h"
#include <stdio.h>

struct FunctionTable;

struct CodegenContext
{
	int uniqueLabelId;
	int stackDepth;

	FunctionTable *funcTable;

	int currentLoopUniqueId;
	usize currentLoopDeferFloor;

	Type currentReturnType;

	BumpArray<GenerateCStringLiteral> cstringLiterals;
	BumpArray<GenerateStringLiteral> stringLiterals;

	BumpArray<Node *> deferStack;
};

void Generate_x86_64(Node *_program,
					 FILE *out,
					 CodegenContext *context);
