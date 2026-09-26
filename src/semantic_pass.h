#pragma once

#include "base/base.h"
#include "parser.h"

struct SymbolTable;
struct FunctionTable;
struct TypeTable;
struct ConstantsTable;
struct MacroTable;

struct SemanticContext
{
	bool hadError;

	bool suppressErrors;

	SymbolTable    *symTable;
	SymbolTable    *globalTable;
	FunctionTable  *funcTable;
	TypeTable      *typeTable;
	ConstantsTable *constTable;
	MacroTable     *macroTable;

	ProcDeclNode *currentFunction;

	Node *currentLoop;

	dynamic_array<GenerateCStringLiteral> cstringLiterals;
	dynamic_array<GenerateStringLiteral> stringLiterals;

	Arena *arenaForAst;

	int macroInstantiationUniqueId;
};

void SemanticPass(Node *_program,
				  SemanticContext *context,
				  Arena *arena);

void AnalyzeExpression(Node *baseNode,
					   SemanticContext *context,
					   Type expectedType = {});
