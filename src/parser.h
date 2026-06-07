#pragma once

#include "common.h"
#include "lexer.h"

#define NODE_TYPE_LIST(X)    \
	X(NodeType_Add,      0)  \
	X(NodeType_Subtract, 1)  \
	X(NodeType_Multiply, 2)  \
	X(NodeType_Divide,   3)  \
	X(NodeType_Number,   4)  \
	X(NodeType_Var,      5)  \
	X(NodeType_Assign,   6)  \
	X(NodeType_VarDecl,  7)  \
	X(NodeType_Block,    8)  \
	X(NodeType_Less,     9)  \
	X(NodeType_Greater,  10) \
	X(NodeType_Equal,    11) \
	X(NodeType_If,       12) \
	X(NodeType_While,    13)

DEFINE_ENUM_WITH_VALUES(NodeType, u32, NODE_TYPE_LIST);

struct AstNode
{
	NodeType type;
	AstNode *lhs;
	AstNode *rhs;

	int numberValue;

	string name;

	AstNode **statements;
	int numStatements;

	AstNode *condition;
	AstNode *thenBlock;
	AstNode *elseBlock;
};

struct Parser
{
	Token current;
	bool hadError;
};

AstNode *ParseExpression(Parser *parser,
						 Lexer *lexer,
						 int minBindingPower,
						 Arena *arena);
AstNode *ParseProgram(Parser *parser,
					  Lexer *lexer,
					  Arena *arena);
