#pragma once

#include "common.h"
#include "lexer.h"

#define NODE_TYPE_LIST(X) \
	X(NodeType_, 0)

DEFINE_ENUM_WITH_VALUES(NodeType, u32, NODE_TYPE_LIST);

struct AstNode
{
	NodeType type;
	AstNode *lhs;
	AstNode *rhs;
};

struct Parser
{
	Token current;
};

AstNode *ParseExpression(Parser *parser,
						 Lexer *lexer,
						 int minBindingPower);
