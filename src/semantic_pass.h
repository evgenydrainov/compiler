#pragma once

#include "common.h"
#include "parser.h"

struct SymbolTable;
struct FunctionTable;
struct TypeTable;

struct SemanticContext
{
	bool hadError;

	SymbolTable *symTable;
	FunctionTable *funcTable;
	TypeTable *typeTable;

	FuncNode *currentFunction;
};

void SemanticPass(Node *_program,
				  SemanticContext *context,
				  Arena *arena);
