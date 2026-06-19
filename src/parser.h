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

//enum BinaryOp : u32
//{
//	BinaryOp_Add,
//	BinaryOp_Subtract,
//	BinaryOp_Multiply,
//	BinaryOp_Divide,
//	BinaryOp_Less,
//	BinaryOp_Greater,
//	BinaryOp_EqualEqual,
//	BinaryOp_LessEqual,
//	BinaryOp_GreaterEqual,
//	BinaryOp_NotEqual,
//};

struct Node
{
	NodeKind kind;
	int line;

	Type inferredType;
};

struct BinaryNode : public Node
{
	// static constexpr NodeKind KIND = NodeKind_Binary;

	Node *lhs;
	Node *rhs;
};

struct AssignNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Assign;

	Node *lhs;
	Node *rhs;
};

struct VarDeclNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_VarDecl;

	string name;
	int stackOffset;
	Node *expr;
	Type type;
};

struct VarNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Var;

	string name;
	int stackOffset;
};

struct NumberNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Number;

	i64 int64Value;
};

struct BlockNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Block;

	Node **statements;
	int numStatements;
	int stackSize;
};

struct IfNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_If;

	Node *condition;
	Node *thenBlock;
	Node *elseBlock;
};

struct WhileNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_While;

	Node *condition;
	Node *body;
};

struct PrintNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Print;

	Node *expr;
};

struct FuncNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Func;

	string name;
	Node *body;
	Type returnType;

	Node **params;
	int numParams;
};

struct CallNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Call;

	string name;

	Node **expressions;
	int numExpressions;
};

struct ReturnNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Return;

	Node *expr;
};

struct ParamNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Param;

	string name;
	int stackOffset;
	Type type;
};

struct BoolNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Bool;

	bool boolValue;
};

struct AddressOfNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_AddressOf;

	Node *what;
};

struct DerefNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Deref;

	Node *what;
};

template <typename T>
inline T *
As(Node *node)
{
	T *result = nullptr;

	if constexpr (IsSameType<T, BinaryNode>::value)
	{
		if (node->kind == NodeKind_Add
			|| node->kind == NodeKind_Subtract
			|| node->kind == NodeKind_Multiply
			|| node->kind == NodeKind_Divide
			|| node->kind == NodeKind_Less
			|| node->kind == NodeKind_Greater
			|| node->kind == NodeKind_EqualEqual
			|| node->kind == NodeKind_LessEqual
			|| node->kind == NodeKind_GreaterEqual
			|| node->kind == NodeKind_NotEqual)
		{
			result = static_cast<T *>(node);
		}
	}
	else
	{
		if (node->kind == T::KIND)
		{
			result = static_cast<T *>(node);
		}
	}

	return result;
}

struct Parser
{
	Token current;
	bool hadError;
};

Node *ParseProgram(Parser *parser,
				   Lexer *lexer,
				   Arena *arena);
