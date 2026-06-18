#pragma once

#include "common.h"
#include "lexer.h"
#include "type.h"

#define NODE_KIND_LIST(X)    \
	X(NodeKind_Add,              0,      "add"          )  \
	X(NodeKind_Subtract,         1,      "subtract"     )  \
	X(NodeKind_Multiply,         2,      "multiply"     )  \
	X(NodeKind_Divide,           3,      "divide"       )  \
	X(NodeKind_Less,             4,      ""             )  \
	X(NodeKind_Greater,          5,      ""             )  \
	X(NodeKind_EqualEqual,       6,      ""             )  \
	X(NodeKind_LessEqual,        7,      ""             )  \
	X(NodeKind_GreaterEqual,     8,      ""             )  \
	X(NodeKind_NotEqual,         9,      ""             )  \
	X(NodeKind_Number,           10,     ""             ) \
	X(NodeKind_Var,              11,     ""             ) \
	X(NodeKind_Assign,           12,     ""             ) \
	X(NodeKind_VarDecl,          13,     ""             ) \
	X(NodeKind_Block,            14,     ""             ) \
	X(NodeKind_If,               15,     ""             ) \
	X(NodeKind_While,            16,     ""             ) \
	X(NodeKind_Print,            17,     ""             ) \
	X(NodeKind_Func,             18,     ""             ) \
	X(NodeKind_Call,             19,     ""             ) \
	X(NodeKind_Return,           20,     ""             ) \
	X(NodeKind_Param,            21,     ""             ) \
	X(NodeKind_Bool,             22,     ""             ) \
	X(NodeKind_AddressOf,        23,     ""             ) \
	X(NodeKind_Deref,            24,     ""             )

DEFINE_ENUM_WITH_VALUES(NodeKind, u32, NODE_KIND_LIST);

struct AstNode
{
	NodeKind kind;
	int line;

	Type inferredType;

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
			AstNode *expr;
			Type type;
		} varDecl;

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
			Type returnType;

			AstNode **params;
			int numParams;
		} func;

		struct
		{
			string name;

			AstNode **expressions;
			int numExpressions;
		} call;

		struct
		{
			AstNode *expr;
		} ret;

		struct
		{
			string name;
			int stackOffset;
			Type type;
		} param;

		struct
		{
			bool value;
		} _bool;

		struct
		{
			AstNode *what;
		} addressOf;

		struct
		{
			AstNode *what;
		} deref;
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
