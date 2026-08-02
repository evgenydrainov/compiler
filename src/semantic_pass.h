#pragma once

#include "common.h"
#include "parser.h"

struct SymbolTable;
struct FunctionTable;
struct TypeTable;
struct ConstantsTable;

struct SemanticContext
{
	bool hadError;

	bool suppressErrors;

	SymbolTable    *symTable;
	SymbolTable    *globalTable;
	FunctionTable  *funcTable;
	TypeTable      *typeTable;
	ConstantsTable *constTable;

	FuncNode *currentFunction;

	Node *currentLoop;

	BumpArray<GenerateCStringLiteral> cstringLiterals;
	BumpArray<GenerateStringLiteral> stringLiterals;
};

void SemanticPass(Node *_program,
				  SemanticContext *context,
				  Arena *arena);

void AnalyzeExpression(Node *baseNode,
					   SemanticContext *context,
					   Type expectedType = {});
