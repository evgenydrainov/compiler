#pragma once

#include "common.h"
#include "lexer.h"
#include "type.h"

#define NODE_KIND_LIST(X) \
	X(NodeKind_Binary,           0,     ""   ) \
	X(NodeKind_Number,           1,     ""   ) \
	X(NodeKind_Var,              2,     ""   ) \
	X(NodeKind_Assign,           3,     ""   ) \
	X(NodeKind_VarDecl,          4,     ""   ) \
	X(NodeKind_Block,            5,     ""   ) \
	X(NodeKind_If,               6,     ""   ) \
	X(NodeKind_While,            7,     ""   ) \
	X(NodeKind_Print,            8,     ""   ) \
	X(NodeKind_Func,             9,     ""   ) \
	X(NodeKind_Call,             10,    ""   ) \
	X(NodeKind_Return,           11,    ""   ) \
	X(NodeKind_Param,            12,    ""   ) \
	X(NodeKind_Bool,             13,    ""   ) \
	X(NodeKind_AddressOf,        14,    ""   ) \
	X(NodeKind_Deref,            15,    ""   ) \
	X(NodeKind_StructDecl,       16,    ""   ) \
	X(NodeKind_StructFieldDecl,  17,    ""   ) \
	X(NodeKind_FieldAccess,      18,    ""   )

DEFINE_ENUM_WITH_VALUES(NodeKind, u32, NODE_KIND_LIST);

#define BINARY_OP_LIST(X) \
	X(BinaryOp_Add,            0,    ""  ) \
	X(BinaryOp_Subtract,       1,    ""  ) \
	X(BinaryOp_Multiply,       2,    ""  ) \
	X(BinaryOp_Divide,         3,    ""  ) \
	X(BinaryOp_Modulo,         4,    ""  ) \
	X(BinaryOp_Less,           5,    ""  ) \
	X(BinaryOp_Greater,        6,    ""  ) \
	X(BinaryOp_EqualEqual,     7,    ""  ) \
	X(BinaryOp_LessEqual,      8,    ""  ) \
	X(BinaryOp_GreaterEqual,   9,    ""  ) \
	X(BinaryOp_NotEqual,       10,   ""  ) \
	X(BinaryOp_LogicalAnd,     11,   ""  ) \
	X(BinaryOp_LogicalOr,      12,   ""  ) \
	X(BinaryOp_BitAnd,         13,   ""  ) \
	X(BinaryOp_BitOr,          14,   ""  ) \
	X(BinaryOp_BitXor,         15,   ""  ) \
	X(BinaryOp_ShiftLeft,      16,   ""  ) \
	X(BinaryOp_ShiftRight,     17,   ""  )

DEFINE_ENUM_WITH_VALUES(BinaryOp, u32, BINARY_OP_LIST);

struct Node
{
	NodeKind kind;
	int line;

	Type inferredType;
};

struct BinaryNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Binary;

	BinaryOp op;
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

	BumpArray<Node *> statements;
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

	bool isForeign;
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

struct StructFieldDeclNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_StructFieldDecl;

	string name;
	Type type;
};

struct StructDeclNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_StructDecl;

	string name;
	BumpArray<StructFieldDeclNode *> fields;
};

struct FieldAccessNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_FieldAccess;

	Node *expr;
	string fieldName;
	int fieldOffset;
};

template <typename T>
inline T *
As(Node *node)
{
	Assert(node->kind == T::KIND);
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
