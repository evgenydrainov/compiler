#pragma once

#include "base/base.h"
#include "parser.h"
#include <stdio.h>

struct CodegenContext
{
	FILE *out;

	int uniqueLabelId;
	int stackDepth;

	int currentLoopUniqueId;
	usize currentLoopDeferFloor;

	Type currentReturnType;

	slice<GenerateCStringLiteral> cstringLiterals;
	slice<GenerateStringLiteral> stringLiterals;

	dynamic_array<Node *> deferStack;
};

void Generate_x86_64(Node *_program,
					 CodegenContext *context);
