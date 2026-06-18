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

	FuncNode *currentFunction;
};

void SemanticPass(Node *_program,
				  SemanticContext *context,
				  Arena *arena);
