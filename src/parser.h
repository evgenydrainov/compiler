#pragma once

#include "common.h"
#include "lexer.h"

#define NODE_TYPE_LIST(X)   \
	X(NodeType_Add,      0) \
	X(NodeType_Subtract, 1) \
	X(NodeType_Multiply, 2) \
	X(NodeType_Divide,   3) \
	X(NodeType_Number,   4)

DEFINE_ENUM_WITH_VALUES(NodeType, u32, NODE_TYPE_LIST);

struct AstNode
{
	NodeType type;
	AstNode *lhs;
	AstNode *rhs;
	int numberValue;
};

struct Parser
{
	Token current;
};

AstNode *ParseExpression(Parser *parser,
						 Lexer *lexer,
						 int minBindingPower,
						 Arena *arena);
