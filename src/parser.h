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

struct Node
{
	NodeKind kind;
	int line;

	Type inferredType;
};

struct BinaryNode : public Node
{
	Node *lhs;
	Node *rhs;
};

struct AssignNode : public Node
{
	string name;
	int stackOffset;
	Node *expr;
};

struct VarDeclNode : public Node
{
	string name;
	int stackOffset;
	Node *expr;
	Type type;
};

struct VarNode : public Node
{
	string name;
	int stackOffset;
};

struct NumberNode : public Node
{
	i64 int64Value;
};

struct BlockNode : public Node
{
	Node **statements;
	int numStatements;
	int stackSize;
};

struct IfNode : public Node
{
	Node *condition;
	Node *thenBlock;
	Node *elseBlock;
};

struct WhileNode : public Node
{
	Node *condition;
	Node *body;
};

struct PrintNode : public Node
{
	Node *expr;
};

struct FuncNode : public Node
{
	string name;
	Node *body;
	Type returnType;

	Node **params;
	int numParams;
};

struct CallNode : public Node
{
	string name;

	Node **expressions;
	int numExpressions;
};

struct ReturnNode : public Node
{
	Node *expr;
};

struct ParamNode : public Node
{
	string name;
	int stackOffset;
	Type type;
};

struct BoolNode : public Node
{
	bool boolValue;
};

struct AddressOfNode : public Node
{
	Node *what;
};

struct DerefNode : public Node
{
	Node *what;
};

template <typename T>
inline T *
As(Node *node)
{
	return static_cast<T *>(node);
}

struct Parser
{
	Token current;
	bool hadError;
};

Node *ParseProgram(Parser *parser,
					  Lexer *lexer,
					  Arena *arena);
