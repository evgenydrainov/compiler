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

	Node *currentFunction;
};

void SemanticPass(Node *program,
				  SemanticContext *context,
				  Arena *arena);
