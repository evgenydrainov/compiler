#pragma once

#include "common.h"
#include "lexer.h"

#define NODE_TYPE_LIST(X)    \
	X(NodeType_Add,          0)  \
	X(NodeType_Subtract,     1)  \
	X(NodeType_Multiply,     2)  \
	X(NodeType_Divide,       3)  \
	X(NodeType_Less,         4)  \
	X(NodeType_Greater,      5)  \
	X(NodeType_EqualEqual,   6)  \
	X(NodeType_LessEqual,    7)  \
	X(NodeType_GreaterEqual, 8)  \
	X(NodeType_NotEqual,     9)  \
	X(NodeType_Number,       10) \
	X(NodeType_Var,          11) \
	X(NodeType_Assign,       12) \
	X(NodeType_VarDecl,      13) \
	X(NodeType_Block,        14) \
	X(NodeType_If,           15) \
	X(NodeType_While,        16) \
	X(NodeType_Print,        17) \
	X(NodeType_Func,         18) \
	X(NodeType_Call,         19)

DEFINE_ENUM_WITH_VALUES(NodeType, u32, NODE_TYPE_LIST);

struct AstNode
{
	NodeType type;
	int line;

	union
	{
		struct
		{
			AstNode *lhs;
			AstNode *rhs;
		} binary;

		struct
		{
			string name;
			int stackOffset;
			AstNode *expr;
		} assign;

		struct
		{
			string name;
			int stackOffset;
		} var;

		struct
		{
			int value;
		} number;

		struct
		{
			AstNode **statements;
			int numStatements;
			int stackSize;
		} block;

		struct
		{
			AstNode *condition;
			AstNode *thenBlock;
			AstNode *elseBlock;
		} _if;

		struct
		{
			AstNode *condition;
			AstNode *body;
		} _while;

		struct
		{
			AstNode *expr;
		} print;

		struct
		{
			string name;
			AstNode *body;
		} func;

		struct
		{
			string name;
		} call;
	};
};

struct Parser
{
	Token current;
	bool hadError;
};

AstNode *ParseProgram(Parser *parser,
					  Lexer *lexer,
					  Arena *arena);
