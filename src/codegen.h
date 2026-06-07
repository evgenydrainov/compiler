#pragma once

#include "common.h"
#include "parser.h"
#include <stdio.h>

struct CodegenContext
{
	int uniqueLabelId;
};

void Generate_x86_64(AstNode *root,
					 FILE *out,
					 CodegenContext *context);
