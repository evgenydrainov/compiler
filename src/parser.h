#pragma once

#include "base/base.h"
#include "lexer.h"
#include "type.h"
#include "ast.h"

struct Parser
{
	Token current;
	bool hadError;

	int uniqueLabelId;

	int numInsertSemicolons;
};

struct GenerateCStringLiteral
{
	string value;
	int uniqueLabelId;
};

struct GenerateStringLiteral
{
	string value;
	int uniqueLabelId;
};

Node *ParseProgram(Parser *parser, Lexer *lexer, Arena *arena);
