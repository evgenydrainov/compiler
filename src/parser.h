#pragma once

#include "base/base.h"
#include "lexer.h"
#include "type.h"

#define NODE_KIND_LIST(X) \
	X(Binary) \
	X(Int64Literal) \
	X(Float32Literal) \
	X(Float64Literal) \
	X(Var) \
	X(Assign) \
	X(VarDecl) \
	X(Block) \
	X(If) \
	X(While) \
	X(Print) \
	X(Func) \
	X(Call) \
	X(Return) \
	X(Param) \
	X(Bool) \
	X(AddressOf) \
	X(Deref) \
	X(StructDecl) \
	X(StructFieldDecl) \
	X(FieldAccess) \
	X(ArrayIndexAccess) \
	X(String) \
	X(CString) \
	X(Unary) \
	X(Asm) \
	X(EnumDecl) \
	X(EnumeratorDecl) \
	X(Cast) \
	X(ConstantDecl) \
	X(Yield) \
	X(Break) \
	X(Continue) \
	X(For) \
	X(Defer) \
	X(Switch) \
	X(Case) \
	X(NullLiteral) \
	X(Sizeof) \
	X(MacroDecl) \
	X(ProcRef) \
	X(MacroRef) \
	X(Proxy)

#define GENERATE_NODE_KIND_ENUM(Name) NodeKind_##Name,

enum NodeKind : u32
{
	NODE_KIND_LIST(GENERATE_NODE_KIND_ENUM)
};

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

enum UnaryOp : u32
{
	UnaryOp_Negate,
	UnaryOp_LogicalNot,
	UnaryOp_BitNegate,
};

struct Node
{
	NodeKind kind;
	SourceLocation location;

	Type inferredType;

	int paramCopyOffset;

	Node *next;
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

	// u8 dummy[64];
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

	list<Node> statements;

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

struct ParamNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Param;

	string name;
	int stackOffset;
	Type type;
};

struct FuncNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Func;

	string name;
	string linkName;

	Node *body;
	Type returnType;

	slice<ParamNode *> params;

	bool isForeign;

	bool isCoroutine;
	int yieldIndex;

	bool isVariadic;
};

struct CallNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Call;

	Node *callee;

	slice<Node *> arguments;

	int returnSlotOffset;
	
	ProcInfo *signature;
	int calleeSlotOffset;
};

struct ReturnNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Return;

	Node *expr;
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
	bump_array<StructFieldDeclNode *> fields;
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
	bump_array<EnumeratorDeclNode *> enumerators;
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

struct DeferNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Defer;

	Node *what;
};

struct CaseNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Case;

	Node *label;
	Node *body;
	i64 labelValue;
};

struct SwitchNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Switch;

	Node *expr;
	bump_array<CaseNode *> cases;
	Node *defaultBody;
};

struct NullLiteralNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_NullLiteral;
};

struct SizeofNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Sizeof;

	Node *what;
};

struct MacroDeclNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_MacroDecl;

	string name;
	Node *body;

	static_bump_array<ParamNode *, 32> params;

	bool dontBind;
	bool isExpressionMacro;
};

struct ProcRefNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_ProcRef;

	string linkName;
};

struct Macro;

struct MacroRefNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_MacroRef;

	Macro *macro;
};

struct ProxyNode : public Node
{
	static constexpr NodeKind KIND = NodeKind_Proxy;

	Node *proxy;
};

template <typename T>
inline T *
As(Node *node)
{
	Assert(node->kind == T::KIND);
	return static_cast<T *>(node);
}

#define GENERATE_NODE_KIND_ENUM_SIZE(Name) case NodeKind_##Name: return sizeof(Name##Node);

inline usize
SizeOfNode(NodeKind kind)
{
	switch (kind)
	{
		NODE_KIND_LIST(GENERATE_NODE_KIND_ENUM_SIZE)
	}
	Assert(false);
	return 0;
}

struct CompileOptions;

struct Parser
{
	Token current;
	bool hadError;

	int uniqueLabelId;

	int numInsertSemicolons;

	CompileOptions *options;
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

template <typename T>
inline T *
MakeNode(SourceLocation location,
		 Arena *arena)
{
	T *node = push_struct<T>(arena);
	node->kind = T::KIND;
	node->location = location;

	return node;
}

inline BinaryNode *
MakeBinaryNode(BinaryOp op,
			   SourceLocation location,
			   Node *lhs,
			   Node *rhs,
			   Arena *arena)
{
	BinaryNode *node = MakeNode<BinaryNode>(location, arena);
	node->op = op;
	node->lhs = lhs;
	node->rhs = rhs;

	return node;
}

inline Int64LiteralNode *
MakeInt64Literal(SourceLocation location,
				 i64 value,
				 Arena *arena)
{
	Int64LiteralNode *node = MakeNode<Int64LiteralNode>(location, arena);
	node->value = value;

	return node;
}

inline VarNode *
MakeVarNode(SourceLocation location,
			string varName,
			Arena *arena)
{
	VarNode *node = MakeNode<VarNode>(location, arena);
	node->name = varName;
	return node;
}

inline VarDeclNode *
MakeVarDeclNodeInfer(SourceLocation location,
					 string varName,
					 Node *expr,
					 Arena *arena)
{
	VarDeclNode *node = MakeNode<VarDeclNode>(location, arena);
	node->name = varName;
	node->expr = expr;
	node->type.kind = TypeKind_InferMe;
	return node;
}

inline AssignNode *
MakeAssignNode(SourceLocation location,
			   Node *lhs,
			   Node *rhs,
			   Arena *arena)
{
	AssignNode *node = MakeNode<AssignNode>(location, arena);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}
