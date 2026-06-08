#pragma once

#include "common.h"
#include "lexer.h"

#define NODE_TYPE_LIST(X)    \
	X(NodeType_Add,          0)  \
	X(NodeType_Subtract,     1)  \
	X(NodeType_Multiply,     2)  \
	X(NodeType_Divide,       3)  \
	X(NodeType_Number,       4)  \
	X(NodeType_Var,          5)  \
	X(NodeType_Assign,       6)  \
	X(NodeType_VarDecl,      7)  \
	X(NodeType_Block,        8)  \
	X(NodeType_Less,         9)  \
	X(NodeType_Greater,      10) \
	X(NodeType_EqualEqual,   11) \
	X(NodeType_If,           12) \
	X(NodeType_While,        13) \
	X(NodeType_LessEqual,    14) \
	X(NodeType_GreaterEqual, 15) \
	X(NodeType_NotEqual,     16)

DEFINE_ENUM_WITH_VALUES(NodeType, u32, NODE_TYPE_LIST);

struct AstNode
{
	NodeType type;
	int line;

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

AstNode *ParseProgram(Parser *parser,
					  Lexer *lexer,
					  Arena *arena);
