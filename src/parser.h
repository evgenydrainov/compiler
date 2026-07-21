#pragma once

#include "common.h"
#include "lexer.h"
#include "type.h"

#define NODE_KIND_LIST(X) \
	X(NodeKind_Binary,            0,     ""   ) \
	X(NodeKind_Int64Literal,      1,     ""   ) \
	X(NodeKind_Float32Literal,    2,     ""   ) \
	X(NodeKind_Float64Literal,    3,     ""   ) \
	X(NodeKind_Var,               4,     ""   ) \
	X(NodeKind_Assign,            5,     ""   ) \
	X(NodeKind_VarDecl,           6,     ""   ) \
	X(NodeKind_Block,             7,     ""   ) \
	X(NodeKind_If,                8,     ""   ) \
	X(NodeKind_While,             9,     ""   ) \
	X(NodeKind_Print,             10,    ""   ) \
	X(NodeKind_Func,              11,    ""   ) \
	X(NodeKind_Call,              12,    ""   ) \
	X(NodeKind_Return,            13,    ""   ) \
	X(NodeKind_Param,             14,    ""   ) \
	X(NodeKind_Bool,              15,    ""   ) \
	X(NodeKind_AddressOf,         16,    ""   ) \
	X(NodeKind_Deref,             17,    ""   ) \
	X(NodeKind_StructDecl,        18,    ""   ) \
	X(NodeKind_StructFieldDecl,   19,    ""   ) \
	X(NodeKind_FieldAccess,       20,    ""   ) \
	X(NodeKind_ArrayIndexAccess,  21,    ""   ) \
	X(NodeKind_String,            22,    ""   ) \
	X(NodeKind_CString,           23,    ""   ) \
	X(NodeKind_Unary,             24,    ""   ) \
	X(NodeKind_Asm,               25,    ""   ) \
	X(NodeKind_EnumDecl,          26,    ""   ) \
	X(NodeKind_EnumeratorDecl,    27,    ""   ) \
	X(NodeKind_Cast,              28,    ""   ) \
	X(NodeKind_ConstantDecl,      29,    ""   ) \
	X(NodeKind_Yield,             30,    ""   ) \
	X(NodeKind_Break,             31,    ""   ) \
	X(NodeKind_Continue,          32,    ""   ) \
	X(NodeKind_For,               33,    ""   )

DEFINE_ENUM_WITH_VALUES(NodeKind, u32, NODE_KIND_LIST);

#define BINARY_OP_LIST(X) \
	X(BinaryOp_Add,            0,    "add"          ) \
	X(BinaryOp_Subtract,       1,    "subtract"     ) \
	X(BinaryOp_Multiply,       2,    "multiply"     ) \
	X(BinaryOp_Divide,         3,    "divide"       ) \
	X(BinaryOp_Modulo,         4,    "modulo"       ) \
	X(BinaryOp_Less,           5,    ""             ) \
	X(BinaryOp_Greater,        6,    ""             ) \
	X(BinaryOp_EqualEqual,     7,    ""             ) \
	X(BinaryOp_LessEqual,      8,    ""             ) \
	X(BinaryOp_GreaterEqual,   9,    ""             ) \
	X(BinaryOp_NotEqual,       10,   ""             ) \
	X(BinaryOp_LogicalAnd,     11,   ""             ) \
	X(BinaryOp_LogicalOr,      12,   ""             ) \
	X(BinaryOp_BitAnd,         13,   "bit-and"      ) \
	X(BinaryOp_BitOr,          14,   "bit-or"       ) \
	X(BinaryOp_BitXor,         15,   "bit-xor"      ) \
	X(BinaryOp_ShiftLeft,      16,   "shift-left"   ) \
	X(BinaryOp_ShiftRight,     17,   "shift-right"  )

DEFINE_ENUM_WITH_VALUES(BinaryOp, u32, BINARY_OP_LIST);

#define UNARY_OP_LIST(X) \
	X(UnaryOp_Negate,          0,    ""  ) \
	X(UnaryOp_LogicalNot,      1,    ""  ) \
	X(UnaryOp_BitNegate,       2,    ""  )

DEFINE_ENUM_WITH_VALUES(UnaryOp, u32, UNARY_OP_LIST);

struct Node
{
	NodeKind kind;
	SourceLocation location;

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

	Node *lhs; // target (dest)
	Node *rhs; // source
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
	bool isGlobal;
};

struct Int64LiteralNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Int64Literal;

	i64 value;
};

struct Float32LiteralNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Float32Literal;

	f32 value;
};

struct Float64LiteralNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Float64Literal;

	f64 value;
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
	string foreignLinkName;

	bool isCoroutine;
	int yieldIndex;

	bool isVariadic;
};

struct CallNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Call;

	string name;

	Node **expressions;
	int numExpressions;

	string linkName;
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

struct ArrayIndexAccessNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_ArrayIndexAccess;

	Node *arrayExpr;
	Node *indexExpr;
};

struct StringNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_String;

	string value;
	int uniqueId;
};

struct CStringNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_CString;

	string value;
	int uniqueId;
};

struct UnaryNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Unary;

	UnaryOp op;
	Node *expr;
};

struct AsmNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Asm;

	string code;
};

struct EnumeratorDeclNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_EnumeratorDecl;

	string name;
};

struct EnumDeclNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_EnumDecl;

	string name;
	BumpArray<EnumeratorDeclNode *> enumerators;
};

struct CastNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Cast;

	Node *what;
	Type targetType;
};

struct ConstantDeclNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_ConstantDecl;

	string name;
	Node *expr;
};

struct YieldNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Yield;

	int yieldIndex;
};

struct BreakNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Break;
};

struct ContinueNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Continue;
};

struct ForNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_For;

	Node *init;
	Node *cond;
	Node *incr;
	Node *body;
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

	int uniqueLabelId;

	int numInsertSemicolons;
};

Node *ParseProgram(Parser *parser,
				   Lexer *lexer,
				   Arena *arena);

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
