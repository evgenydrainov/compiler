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

	AstNode *currentFunction;
};

void SemanticPass(AstNode *program,
				  SemanticContext *context,
				  Arena *arena);
