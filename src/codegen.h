#pragma once

#include "common.h"
#include "parser.h"
#include <stdio.h>

struct CodegenContext
{
	int uniqueLabelId;
	int stackDepth;

	BumpArray<GenerateCStringLiteral> cstringLiterals;
	BumpArray<GenerateStringLiteral> stringLiterals;
};

void Generate_x86_64(Node *_program,
					 FILE *out,
					 CodegenContext *context);
