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

	bump_array<GenerateCStringLiteral> cstringLiterals;
	bump_array<GenerateStringLiteral> stringLiterals;

	bump_array<Node *> deferStack;
};

void Generate_x86_64(Node *_program,
					 CodegenContext *context);
