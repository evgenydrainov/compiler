#pragma once

#include "common.h"
#include "parser.h"

struct SemanticPassContext
{
	bool hadError;
};

void SemanticPass(AstNode *program,
				  SemanticPassContext *context);
