#pragma once

#include "common.h"
#include "parser.h"

struct SymbolTable;
struct FunctionTable;

struct SemanticContext
{
	bool hadError;
	SymbolTable *symTable;
	FunctionTable *funcTable;
};

void SemanticPass(AstNode *program,
				  SemanticContext *context);
