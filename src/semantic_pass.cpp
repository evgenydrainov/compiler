#include "semantic_pass.h"

#include "symbol_table.h"
#include "function_table.h"
#include "type_table.h"
#include "constants_table.h"
#include "macro_table.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

internal void
Error(SemanticContext *context,
	  Node *node,
	  char *format,
	  ...)
{
	va_list args;
	va_start(args, format);

	if (!context->suppressErrors)
	{
		fprintf(stderr, STR_FMT "(%d, %d): error: ",
				STR_ARG(node->location.fileName), node->location.line, node->location.column);
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
	}

	va_end(args);

	context->hadError = true;
}

internal void
Note(SemanticContext *context,
	 Node *node,
	 char *format,
	 ...)
{
	va_list args;
	va_start(args, format);

	if (!context->suppressErrors)
	{
		fprintf(stderr, STR_FMT "(%d, %d): note: ",
				STR_ARG(node->location.fileName), node->location.line, node->location.column);
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
	}

	va_end(args);
}

struct Scope
{
	int numSymbols;
	int stackSize;
	int scopeStart;
};

internal Scope
EnterScope(SymbolTable *symTable)
{
	Scope scope = {};

	scope.numSymbols = symTable->count;
	scope.stackSize = symTable->stackSize;
	scope.scopeStart = symTable->scopeStart;

	symTable->scopeStart = symTable->count;

	return scope;
}

internal void
LeaveScope(SymbolTable *symTable, Scope scope)
{
	symTable->count = scope.numSymbols;
	symTable->stackSize = scope.stackSize;
	symTable->scopeStart = scope.scopeStart;
}

internal void
AnalyzeStatement(Node *node,
				 SemanticContext *context);

internal void
AnalyzeBlock(Node *baseNode,
			 SemanticContext *context)
{
	BlockNode *node = As<BlockNode>(baseNode);

	Scope scope = EnterScope(context->symTable);

	for (Node *it : node->statements)
	{
		AnalyzeStatement(it, context);
	}

	LeaveScope(context->symTable, scope);
}

internal bool
IsLValue(Node *_node)
{
	bool result = false;

	switch (_node->kind)
	{
		case NodeKind_Var:   {result = true;} break;
		case NodeKind_Deref: {result = true;} break;

		case NodeKind_FieldAccess:
		{
			FieldAccessNode *node = As<FieldAccessNode>(_node);
			result = IsLValue(node->expr);
		} break;

		case NodeKind_ArrayIndexAccess:
		{
			ArrayIndexAccessNode *node = As<ArrayIndexAccessNode>(_node);
			result = IsLValue(node->arrayExpr);
		} break;

		case NodeKind_Cast:
		{
			CastNode *node = As<CastNode>(_node);
			result = IsLValue(node->what);
		} break;

		case NodeKind_Proxy:
		{
			ProxyNode *node = As<ProxyNode>(_node);
			result = IsLValue(node->proxy);
		} break;

		default: {} break;
	}

	return result;
}

internal i64
EvaluateConstantExpression(Node *baseNode,
						   SemanticContext *context)
{
	if (baseNode->kind == NodeKind_Int64Literal)
	{
		Int64LiteralNode *node = As<Int64LiteralNode>(baseNode);

		return node->value;
	}
	else if (baseNode->kind == NodeKind_Binary)
	{
		BinaryNode *node = As<BinaryNode>(baseNode);

		i64 value1 = EvaluateConstantExpression(node->lhs, context);
		i64 value2 = EvaluateConstantExpression(node->rhs, context);

		if (node->op == BinaryOp_Divide
			|| node->op == BinaryOp_Modulo)
		{
			if (value2 == 0)
			{
				Error(context, baseNode, "division by zero in constant expression");
				return 0;
			}
		}

		i64 result = 0;

		switch (node->op)
		{
			case BinaryOp_Add:          {result = value1 + value2;} break;
			case BinaryOp_Subtract:     {result = value1 - value2;} break;
			case BinaryOp_Multiply:     {result = value1 * value2;} break;
			case BinaryOp_Divide:       {result = value1 / value2;} break;
			case BinaryOp_Modulo:       {result = value1 % value2;} break;
			case BinaryOp_Less:         {result = value1 < value2;} break;
			case BinaryOp_Greater:      {result = value1 > value2;} break;
			case BinaryOp_EqualEqual:   {result = value1 == value2;} break;
			case BinaryOp_LessEqual:    {result = value1 <= value2;} break;
			case BinaryOp_GreaterEqual: {result = value1 >= value2;} break;
			case BinaryOp_NotEqual:     {result = value1 != value2;} break;
			case BinaryOp_LogicalAnd:   {result = value1 && value2;} break;
			case BinaryOp_LogicalOr:    {result = value1 || value2;} break;
			case BinaryOp_BitAnd:       {result = value1 & value2;} break;
			case BinaryOp_BitOr:        {result = value1 | value2;} break;
			case BinaryOp_BitXor:       {result = value1 ^ value2;} break;
			case BinaryOp_ShiftLeft:    {result = value1 << value2;} break;
			case BinaryOp_ShiftRight:   {result = value1 >> value2;} break;
		}

		return result;
	}
	else if (baseNode->kind == NodeKind_Var)
	{
		VarNode *node = As<VarNode>(baseNode);

		i64 result = 0;

		Constant *constant = LookupConstant(context->constTable, node->name);
		if (constant)
		{
			result = constant->value;
		}
		else
		{
			Error(context, node, STR_FMT_QUOTED " is not a constant", STR_ARG(node->name));
			Note(context, node, "only constants declared with '::' can be used in a constant expression");
		}

		return result;
	}
	else if (baseNode->kind == NodeKind_Unary)
	{
		UnaryNode *node = As<UnaryNode>(baseNode);

		i64 value = EvaluateConstantExpression(node->expr, context);

		i64 result = 0;

		switch (node->op)
		{
			case UnaryOp_Negate:     {result = -value;} break;
			case UnaryOp_LogicalNot: {result = !value;} break;
			case UnaryOp_BitNegate:  {result = ~value;} break;
		}

		return result;
	}
	else
	{
		Error(context, baseNode, "expression is not a compile-time constant");
		Note(context, baseNode, "a constant expression can only use integer literals, constants and operators");
	}

	return 0;
}

internal bool
TryEvaluateConstantExpression(Node *baseNode,
							  SemanticContext *context,
							  i64 *outResult)
{
	bool saveSuppressErrors = context->suppressErrors;
	bool saveHadError = context->hadError;

	context->suppressErrors = true;
	context->hadError = false;

	i64 result = EvaluateConstantExpression(baseNode, context);

	bool evaluationFailed = context->hadError;

	context->suppressErrors = saveSuppressErrors;
	context->hadError = saveHadError;

	if (!evaluationFailed)
	{
		*outResult = result;
		return true;
	}
	else
	{
		*outResult = 0;
		return false;
	}
}

internal void
ResolveType(Type *type,
			SemanticContext *context,
			Node *nodeForError)
{
	if (type->kind == TypeKind_Struct)
	{
		if (!type->structInfo
			&& !type->enumInfo)
		{
			Type *typeInfo = LookupType(context->typeTable, type->name);
			if (typeInfo)
			{
				type->structInfo = typeInfo->structInfo;
				type->enumInfo = typeInfo->enumInfo;

				if (type->enumInfo)
				{
					Assert(!type->structInfo);
					type->kind = TypeKind_Enum;
				}
			}
			else
			{
				Error(context, nodeForError,
					  "undeclared type " STR_FMT_QUOTED " (there is no struct or enum with this name)",
					  STR_ARG(type->name));

				local_persist StructInfo dummyStructInfo;
				type->structInfo = &dummyStructInfo; // avoid crashes
			}
		}
		else
		{
			// type is already resolved
		}
	}
	else if (type->kind == TypeKind_Array)
	{
		ResolveType(type->arrayElementType, context, nodeForError);

		if (type->arrayLength == 0)
		{
			i64 arrayLength = EvaluateConstantExpression(type->arrayLengthExpr, context);
			if (arrayLength > 0)
			{
				type->arrayLength = (int)arrayLength;
			}
			else
			{
				Error(context, nodeForError,
					  "array length must be greater than zero, but it is %lld",
					  arrayLength);

				type->arrayLength = 1; // avoid crash
			}
		}
		else
		{
			// type is already resolved
		}
	}
	else if (type->kind == TypeKind_Pointer)
	{
		ResolveType(type->pointee, context, nodeForError);
	}
	else if (type->kind == TypeKind_Slice
			 || type->kind == TypeKind_DynamicArray)
	{
		ResolveType(type->arrayElementType, context, nodeForError);
	}
	else if (type->kind == TypeKind_Proc)
	{
		Assert(type->procInfo);

		for (usize i = 0;
			 i < type->procInfo->params.count;
			 i++)
		{
			ResolveType(&type->procInfo->params[i], context, nodeForError);
		}

		ResolveType(&type->procInfo->returnType, context, nodeForError);
	}
}

internal bool
CanImplicitlyCast(Type destType,
				  Node *source,
				  SemanticContext *context)
{
	if (destType.kind == TypeKind_Unknown)
	{
		return false;
	}

	if (TypesEqual(destType, source->inferredType))
	{
		return true;
	}

	if (IsSignedInteger(destType))
	{
		i64 value;
		if (TryEvaluateConstantExpression(source, context, &value))
		{
			struct Range
			{
				i64 min;
				i64 max;
			};

			Range range = {};

			if (destType.kind == TypeKind_Int64)
			{
				range = { INT64_MIN, INT64_MAX };
			}
			else if (destType.kind == TypeKind_Int32)
			{
				range = { INT32_MIN, INT32_MAX };
			}
			else if (destType.kind == TypeKind_Int16)
			{
				range = { INT16_MIN, INT16_MAX };
			}
			else if (destType.kind == TypeKind_Int8)
			{
				range = { INT8_MIN, INT8_MAX };
			}
			else
			{
				Assert(false);
			}

			if (value >= range.min && value <= range.max)
			{
				return true;
			}
		}
	}

	if (IsUnsignedInteger(destType))
	{
		i64 value;
		if (TryEvaluateConstantExpression(source, context, &value))
		{
			struct Range
			{
				u64 min;
				u64 max;
			};

			Range range = {};

			if (destType.kind == TypeKind_UInt64)
			{
				range = { 0, UINT64_MAX };
			}
			else if (destType.kind == TypeKind_UInt32)
			{
				range = { 0, UINT32_MAX };
			}
			else if (destType.kind == TypeKind_UInt16)
			{
				range = { 0, UINT16_MAX };
			}
			else if (destType.kind == TypeKind_UInt8)
			{
				range = { 0, UINT8_MAX };
			}
			else
			{
				Assert(false);
			}

			if ((u64)value >= range.min && (u64)value <= range.max)
			{
				return true;
			}
		}
	}

	if (destType.kind == TypeKind_Int64)
	{
		if (source->inferredType.kind == TypeKind_Int32)
		{
			// int32 fits in int64
			return true;
		}
		if (source->inferredType.kind == TypeKind_Int16)
		{
			// int16 fits in int64
			return true;
		}
		if (source->inferredType.kind == TypeKind_Int8)
		{
			// int8 fits in int64
			return true;
		}
	}

	if (destType.kind == TypeKind_Int32)
	{
		if (source->inferredType.kind == TypeKind_Int16)
		{
			// int16 fits in int32
			return true;
		}
		if (source->inferredType.kind == TypeKind_Int8)
		{
			// int8 fits in int32
			return true;
		}
	}

	if (destType.kind == TypeKind_Int16)
	{
		if (source->inferredType.kind == TypeKind_Int8)
		{
			// int8 fits in int16
			return true;
		}
	}

	if (destType.kind == TypeKind_Pointer
		|| destType.kind == TypeKind_Proc)
	{
		if (source->inferredType.kind == TypeKind_Pointer
			&& source->inferredType.pointee->kind == TypeKind_Void)
		{
			// allow *void to cast into any pointer for now
			// foo: *int = null;
			return true;
		}
	}

	if (destType.kind == TypeKind_Pointer
		&& destType.pointee->kind == TypeKind_Void)
	{
		if (source->inferredType.kind == TypeKind_Pointer)
		{
			// allow any pointer to cast into *void for now
			// foo: *void = &my_value;
			return true;
		}
	}

	return false;
}

internal void
AnalyzeBinaryExpression(Node *baseNode,
						SemanticContext *context)
{
	BinaryNode *node = As<BinaryNode>(baseNode);

	switch (node->op)
	{
		default:
		{
			Assert(false);
		} break;

		case BinaryOp_Add:
		case BinaryOp_Subtract:
		case BinaryOp_Multiply:
		case BinaryOp_Divide:
		case BinaryOp_Modulo:
		case BinaryOp_ShiftLeft:
		case BinaryOp_ShiftRight:
		case BinaryOp_BitAnd:
		case BinaryOp_BitOr:
		case BinaryOp_BitXor:
		{
			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context);

			char *opName = GetBinaryOpPrettyName(node->op);

			if (CanImplicitlyCast(node->lhs->inferredType, node->rhs, context)
				&& node->lhs->inferredType.kind != TypeKind_Struct
				&& node->rhs->inferredType.kind != TypeKind_Struct)
			{
				node->inferredType = node->lhs->inferredType;
			}
			else
			{
				Error(context, node, "cannot %s '%s' and '%s'",
					  opName,
					  GetTypeKindPrettyName(node->lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->rhs->inferredType.kind));
			}
		} break;

		case BinaryOp_Less:
		case BinaryOp_Greater:
		case BinaryOp_LessEqual:
		case BinaryOp_GreaterEqual:
		{
			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context);

			if (CanImplicitlyCast(node->lhs->inferredType, node->rhs, context)
				&& node->lhs->inferredType.kind != TypeKind_Struct
				&& node->rhs->inferredType.kind != TypeKind_Struct)
			{
				node->inferredType.kind = TypeKind_Bool;
			}
			else
			{
				Error(context, node, "cannot compare '%s' and '%s'",
					  GetTypeKindPrettyName(node->lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->rhs->inferredType.kind));
			}
		} break;

		case BinaryOp_EqualEqual:
		case BinaryOp_NotEqual:
		{
			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context, node->lhs->inferredType);

			if (CanImplicitlyCast(node->lhs->inferredType, node->rhs, context)
				&& node->lhs->inferredType.kind != TypeKind_Struct
				&& node->rhs->inferredType.kind != TypeKind_Struct)
			{
				node->inferredType.kind = TypeKind_Bool;
			}
			else
			{
				Error(context, node, "cannot compare '%s' and '%s'",
					  GetTypeKindPrettyName(node->lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->rhs->inferredType.kind));
			}
		} break;

		case BinaryOp_LogicalAnd:
		case BinaryOp_LogicalOr:
		{
			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context);

			if (node->lhs->inferredType.kind == TypeKind_Bool
				&& node->rhs->inferredType.kind == TypeKind_Bool)
			{
				node->inferredType.kind = TypeKind_Bool;
			}
			else
			{
				Error(context, node, "both operands must be 'bool', but they are '%s' and '%s'",
					  GetTypeKindPrettyName(node->lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->rhs->inferredType.kind));
			}
		} break;
	}
}

template <typename DestType, typename SrcType>
internal DestType *
ReinterpretNode(SrcType *&node)
{
	static_assert(!is_same<SrcType, Node>::value);

	static_assert(sizeof(DestType) <= sizeof(SrcType));

	SourceLocation location = node->location;
	Node *next = node->next;

	DestType *newNode = (DestType *)node;
	*newNode = {};
	newNode->kind = DestType::KIND;
	newNode->location = location;
	newNode->next = next;

	node = nullptr;

	return newNode;
}

struct InstantiateContext
{
	Arena *arena;
	MacroDeclNode *decl;
	int uniqueId;

	slice<Node *> arguments;
};

internal Node *
InstantiateMacro(Node *baseNode,
				 InstantiateContext *context)
{
	if (!baseNode)
	{
		return nullptr;
	}

	Arena *arena = context->arena;
	MacroDeclNode *decl = context->decl;

	switch (baseNode->kind)
	{
		default:
		{
			Assert(false);

			return nullptr;
		} break;

		case NodeKind_Binary:
		{
			BinaryNode *node = As<BinaryNode>(baseNode);

			BinaryNode *result = MakeNode<BinaryNode>(node->location, arena);
			result->op = node->op;
			result->lhs = InstantiateMacro(node->lhs, context);
			result->rhs = InstantiateMacro(node->rhs, context);

			return result;
		} break;

		case NodeKind_Assign:
		{
			AssignNode *node = As<AssignNode>(baseNode);

			AssignNode *result = MakeNode<AssignNode>(node->location, arena);
			result->lhs = InstantiateMacro(node->lhs, context);
			result->rhs = InstantiateMacro(node->rhs, context);

			return result;
		} break;

		case NodeKind_VarDecl:
		{
			VarDeclNode *node = As<VarDeclNode>(baseNode);

			VarDeclNode *result = MakeNode<VarDeclNode>(node->location, arena);
			result->name = node->name;
			result->expr = InstantiateMacro(node->expr, context);
			result->type = node->type;

			return result;
		} break;

		case NodeKind_Var:
		{
			VarNode *node = As<VarNode>(baseNode);

			if (decl)
			{
				for (int i = 0; i < decl->params.count; i++)
				{
					if (decl->params[i]->name == node->name)
					{
						if (decl->dontBind)
						{
							InstantiateContext context2 = *context;
							context2.decl = nullptr;

							Node *result = InstantiateMacro(context->arguments[i], &context2);
							return result;
						}
						else
						{
							VarNode *result = MakeNode<VarNode>(node->location, arena);
							result->name = tprintf("$arg_%d_%d", i, context->uniqueId);
							return result;
						}
					}
				}
			}

			VarNode *result = MakeNode<VarNode>(node->location, arena);
			result->name = node->name;
			return result;
		} break;

		case NodeKind_Int64Literal:
		{
			Int64LiteralNode *node = As<Int64LiteralNode>(baseNode);

			Int64LiteralNode *result = MakeNode<Int64LiteralNode>(node->location, arena);
			result->value = node->value;

			return result;
		} break;

		case NodeKind_Float32Literal:
		{
			Float32LiteralNode *node = As<Float32LiteralNode>(baseNode);

			Float32LiteralNode *result = MakeNode<Float32LiteralNode>(node->location, arena);
			result->value = node->value;

			return result;
		} break;

		case NodeKind_Float64Literal:
		{
			Float64LiteralNode *node = As<Float64LiteralNode>(baseNode);

			Float64LiteralNode *result = MakeNode<Float64LiteralNode>(node->location, arena);
			result->value = node->value;

			return result;
		} break;

		case NodeKind_Block:
		{
			BlockNode *node = As<BlockNode>(baseNode);

			BlockNode *result = MakeNode<BlockNode>(node->location, arena);
			for (Node *it : node->statements)
			{
				Node *statement = InstantiateMacro(it, context);
				list_append(&result->statements, statement);
			}

			return result;
		} break;

		case NodeKind_If:
		{
			IfNode *node = As<IfNode>(baseNode);

			IfNode *result = MakeNode<IfNode>(node->location, arena);
			result->condition = InstantiateMacro(node->condition, context);
			result->thenBlock = InstantiateMacro(node->thenBlock, context);
			result->elseBlock = InstantiateMacro(node->elseBlock, context);

			return result;
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);

			WhileNode *result = MakeNode<WhileNode>(node->location, arena);
			result->condition = InstantiateMacro(node->condition, context);
			result->body = InstantiateMacro(node->body, context);

			return result;
		} break;

		case NodeKind_Print:
		{
			PrintNode *node = As<PrintNode>(baseNode);

			PrintNode *result = MakeNode<PrintNode>(node->location, arena);
			result->expr = InstantiateMacro(node->expr, context);

			return result;
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(baseNode);

			CallNode *result = MakeNode<CallNode>(node->location, arena);
			result->callee = InstantiateMacro(node->callee, context);
			result->arguments = push_slice<Node *>(arena, node->arguments.count);
			for (usize i = 0; i < node->arguments.count; i++)
			{
				result->arguments[i] = InstantiateMacro(node->arguments[i], context);
			}

			return result;
		} break;

		case NodeKind_Return:
		{
			ReturnNode *node = As<ReturnNode>(baseNode);

			ReturnNode *result = MakeNode<ReturnNode>(node->location, arena);
			result->expr = InstantiateMacro(node->expr, context);

			return result;
		} break;

		case NodeKind_Bool:
		{
			BoolNode *node = As<BoolNode>(baseNode);

			BoolNode *result = MakeNode<BoolNode>(node->location, arena);
			result->boolValue = node->boolValue;

			return result;
		} break;

		case NodeKind_AddressOf:
		{
			AddressOfNode *node = As<AddressOfNode>(baseNode);

			AddressOfNode *result = MakeNode<AddressOfNode>(node->location, arena);
			result->what = InstantiateMacro(node->what, context);

			return result;
		} break;

		case NodeKind_Deref:
		{
			DerefNode *node = As<DerefNode>(baseNode);

			DerefNode *result = MakeNode<DerefNode>(node->location, arena);
			result->what = InstantiateMacro(node->what, context);

			return result;
		} break;

		case NodeKind_FieldAccess:
		{
			FieldAccessNode *node = As<FieldAccessNode>(baseNode);

			FieldAccessNode *result = MakeNode<FieldAccessNode>(node->location, arena);
			result->expr = InstantiateMacro(node->expr, context);
			result->fieldName = node->fieldName;

			return result;
		} break;

		case NodeKind_ArrayIndexAccess:
		{
			ArrayIndexAccessNode *node = As<ArrayIndexAccessNode>(baseNode);

			ArrayIndexAccessNode *result = MakeNode<ArrayIndexAccessNode>(node->location, arena);
			result->arrayExpr = InstantiateMacro(node->arrayExpr, context);
			result->indexExpr = InstantiateMacro(node->indexExpr, context);

			return result;
		} break;

		case NodeKind_String:
		{
			StringNode *node = As<StringNode>(baseNode);

			StringNode *result = MakeNode<StringNode>(node->location, arena);
			result->value = node->value;

			return result;
		} break;

		case NodeKind_CString:
		{
			CStringNode *node = As<CStringNode>(baseNode);

			CStringNode *result = MakeNode<CStringNode>(node->location, arena);
			result->value = node->value;

			return result;
		} break;

		case NodeKind_Unary:
		{
			UnaryNode *node = As<UnaryNode>(baseNode);

			UnaryNode *result = MakeNode<UnaryNode>(node->location, arena);
			result->op = node->op;
			result->expr = InstantiateMacro(node->expr, context);

			return result;
		} break;

		case NodeKind_Cast:
		{
			CastNode *node = As<CastNode>(baseNode);

			CastNode *result = MakeNode<CastNode>(node->location, arena);
			result->what = InstantiateMacro(node->what, context);
			result->targetType = node->targetType;

			return result;
		} break;

		case NodeKind_Yield:
		{
			YieldNode *node = As<YieldNode>(baseNode);

			YieldNode *result = MakeNode<YieldNode>(node->location, arena);

			return result;
		} break;

		case NodeKind_Break:
		{
			BreakNode *node = As<BreakNode>(baseNode);

			BreakNode *result = MakeNode<BreakNode>(node->location, arena);

			return result;
		} break;

		case NodeKind_Continue:
		{
			ContinueNode *node = As<ContinueNode>(baseNode);

			ContinueNode *result = MakeNode<ContinueNode>(node->location, arena);

			return result;
		} break;

		case NodeKind_For:
		{
			ForNode *node = As<ForNode>(baseNode);

			ForNode *result = MakeNode<ForNode>(node->location, arena);
			result->init = InstantiateMacro(node->init, context);
			result->cond = InstantiateMacro(node->cond, context);
			result->incr = InstantiateMacro(node->incr, context);
			result->body = InstantiateMacro(node->body, context);

			return result;
		} break;

		case NodeKind_Defer:
		{
			DeferNode *node = As<DeferNode>(baseNode);

			DeferNode *result = MakeNode<DeferNode>(node->location, arena);
			result->what = InstantiateMacro(node->what, context);

			return result;
		} break;

		case NodeKind_Case:
		{
			CaseNode *node = As<CaseNode>(baseNode);

			CaseNode *result = MakeNode<CaseNode>(node->location, arena);
			result->label = InstantiateMacro(node->label, context);
			result->body = InstantiateMacro(node->body, context);

			return result;
		} break;

		case NodeKind_Switch:
		{
			Assert(false);

			return nullptr;
		} break;

		case NodeKind_NullLiteral:
		{
			NullLiteralNode *node = As<NullLiteralNode>(baseNode);

			NullLiteralNode *result = MakeNode<NullLiteralNode>(node->location, arena);

			return result;
		} break;

		case NodeKind_Sizeof:
		{
			SizeofNode *node = As<SizeofNode>(baseNode);

			SizeofNode *result = MakeNode<SizeofNode>(node->location, arena);
			result->what = InstantiateMacro(node->what, context);

			return result;
		} break;
	}
}

void
AnalyzeExpression(Node *baseNode,
				  SemanticContext *context,
				  Type expectedType)
{
	switch (baseNode->kind)
	{
		default:
		{
			Assert(false);
		} break;

		case NodeKind_Int64Literal:
		{
			baseNode->inferredType.kind = TypeKind_Int64;
		} break;

		case NodeKind_Float32Literal:
		{
			baseNode->inferredType.kind = TypeKind_Float32;
		} break;

		case NodeKind_Float64Literal:
		{
			baseNode->inferredType.kind = TypeKind_Float64;
		} break;

		case NodeKind_NullLiteral:
		{
			local_persist Type voidType = { TypeKind_Void };

			baseNode->inferredType.kind = TypeKind_Pointer;
			baseNode->inferredType.pointee = &voidType;
		} break;

		case NodeKind_String:
		{
			baseNode->inferredType.kind = TypeKind_Struct;
			baseNode->inferredType.name = "string";

			ResolveType(&baseNode->inferredType, context, baseNode);

			StringNode *node = As<StringNode>(baseNode);

			GenerateStringLiteral literal = {};
			literal.value = node->value;
			literal.uniqueLabelId = node->uniqueId;

			array_add(&context->stringLiterals, literal);
		} break;

		case NodeKind_CString:
		{
			baseNode->inferredType.kind = TypeKind_Pointer;

			local_persist Type uint8Type = { TypeKind_UInt8 };
			baseNode->inferredType.pointee = &uint8Type;

			CStringNode *node = As<CStringNode>(baseNode);

			GenerateCStringLiteral literal = {};
			literal.value = node->value;
			literal.uniqueLabelId = node->uniqueId;

			array_add(&context->cstringLiterals, literal);
		} break;

		case NodeKind_Bool:
		{
			baseNode->inferredType.kind = TypeKind_Bool;
		} break;

		case NodeKind_Sizeof:
		{
			SizeofNode *node = As<SizeofNode>(baseNode);

			AnalyzeExpression(node->what, context);

			int size = SizeOfType(node->what->inferredType);

			Int64LiteralNode *newNode = ReinterpretNode<Int64LiteralNode>(node);
			newNode->inferredType.kind = TypeKind_Int64;
			newNode->value = size;
		} break;

		case NodeKind_Binary:
		{
			AnalyzeBinaryExpression(baseNode, context);
		} break;

		case NodeKind_Unary:
		{
			UnaryNode *node = As<UnaryNode>(baseNode);

			AnalyzeExpression(node->expr, context);

			if (node->op == UnaryOp_Negate
				|| node->op == UnaryOp_BitNegate)
			{
				if (IsSignedInteger(node->expr->inferredType)
					|| IsFloatingPoint(node->expr->inferredType))
				{
					node->inferredType = node->expr->inferredType;
				}
				else
				{
					Error(context, node->expr,
						  "cannot negate '%s': operand must be a signed integer or a float",
						  GetTypeKindPrettyName(node->expr->inferredType.kind));
				}
			}
			else if (node->op == UnaryOp_LogicalNot)
			{
				if (node->expr->inferredType.kind == TypeKind_Bool)
				{
					node->inferredType.kind = TypeKind_Bool;
				}
				else
				{
					Error(context, node->expr, "cannot negate '%s': operand must be 'bool'",
						  GetTypeKindPrettyName(node->expr->inferredType.kind));
				}
			}
			else
			{
				Assert(false);
			}
		} break;

		case NodeKind_Var:
		{
			VarNode *node = As<VarNode>(baseNode);

			Symbol *symbol = LookupSymbol(context->symTable, node->name, 0);
			if (symbol)
			{
				node->stackOffset = symbol->stackOffset;
				node->inferredType = symbol->type;

				break;
			}

			Constant *constant = LookupConstant(context->constTable, node->name);
			if (constant)
			{
				Int64LiteralNode *newNode = ReinterpretNode<Int64LiteralNode>(node);
				newNode->inferredType.kind = TypeKind_Int64;
				newNode->value = constant->value;

				break;
			}

			symbol = LookupSymbol(context->globalTable, node->name, 0);
			if (symbol)
			{
				node->inferredType = symbol->type;
				node->isGlobal = true;

				break;
			}

			Function *function = LookupFunction(context->funcTable, node->name);
			if (function)
			{
				ProcRefNode *newNode = ReinterpretNode<ProcRefNode>(node);
				newNode->linkName = function->linkName;
				newNode->inferredType.kind = TypeKind_Proc;
				newNode->inferredType.procInfo = function->info;

				break;
			}

			Macro *macro = LookupMacro(context->macroTable, node->name);
			if (macro)
			{
				if (macro->decl->isExpressionMacro)
				{
					InstantiateContext instContext = {};
					instContext.arena = context->arenaForAst;
					instContext.decl = macro->decl;
					instContext.uniqueId = ++context->macroInstantiationUniqueId;

					Node *instantiated = InstantiateMacro(macro->decl->body, &instContext);

#if 1
					ProxyNode *newNode = ReinterpretNode<ProxyNode>(node);
					newNode->proxy = instantiated;

					AnalyzeExpression(newNode, context, expectedType);
#else
					usize destSize = SizeOfNode(node->kind);
					usize sourceSize = SizeOfNode(instantiated->kind);

					Assert(destSize >= sourceSize);

					SourceLocation location = node->location;
					Node *next = node->next;

					MemCpy(node, instantiated, sourceSize);

					node->location = location;
					node->next = next;

					AnalyzeExpression(node, context, expectedType);
#endif

					break;
				}
				else
				{
					MacroRefNode *newNode = ReinterpretNode<MacroRefNode>(node);
					newNode->macro = macro;

					break;
				}
			}

			Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->name));
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(baseNode);

			AnalyzeExpression(node->callee, context);

			if (node->callee->kind == NodeKind_MacroRef)
			{
				Macro *macro = As<MacroRefNode>(node->callee)->macro;

				InstantiateContext instContext = {};
				instContext.arena = context->arenaForAst;
				instContext.decl = macro->decl;
				instContext.uniqueId = ++context->macroInstantiationUniqueId;
				instContext.arguments = node->arguments;

				list<Node> statements = {};

				if (!macro->decl->dontBind)
				{
					for (int i = 0; i < node->arguments.count; i++)
					{
						string name = tprintf("$arg_%d_%d", i, instContext.uniqueId);

						VarDeclNode *varDecl = MakeNode<VarDeclNode>(node->location, context->arenaForAst);
						varDecl->name = name;
						varDecl->expr = node->arguments[i];
						varDecl->type.kind = TypeKind_InferMe;

						list_append(&statements, (Node *)varDecl);
					}
				}

				{
					Node *instantiated = InstantiateMacro(macro->decl->body, &instContext);
					list_append(&statements, instantiated);
				}

				BlockNode *newNode = ReinterpretNode<BlockNode>(node);
				newNode->statements = statements;

				AnalyzeBlock(newNode, context);

				break;
			}

			if (node->callee->inferredType.kind != TypeKind_Proc)
			{
				Error(context, node->callee, "cannot call a value of type '%s'",
					  GetTypeKindPrettyName(node->callee->inferredType.kind));
				break;
			}

			node->signature = node->callee->inferredType.procInfo;
			node->calleeSlotOffset = ReserveSpace(context->symTable, node->callee->inferredType);

			bool good = ((node->signature->isVariadic)
						 ? (node->arguments.count >= node->signature->params.count)
						 : (node->arguments.count == node->signature->params.count));
			if (!good)
			{
				Error(context, node,
					  "cannot call: expected %s%d argument%s, but %d %s given",
					  node->signature->isVariadic ? "at least " : "",
					  (int)node->signature->params.count,
					  (node->signature->params.count == 1) ? "" : "s",
					  (int)node->arguments.count,
					  (node->arguments.count == 1) ? "was" : "were");
				break;
			}

			for (Node *expr : node->arguments)
			{
				AnalyzeExpression(expr, context);
			}

			for (usize i = 0;
				 i < node->signature->params.count;
				 i++)
			{
				Node *expr = node->arguments[i];

				ResolveType(&node->signature->params[i], context, node);

				if (CanImplicitlyCast(node->signature->params[i], expr, context))
				{
					if (!IsRegisterSized(node->signature->params[i]))
					{
						expr->paramCopyOffset = ReserveSpace(context->symTable, node->signature->params[i]);
					}
				}
				else
				{
					Error(context, expr,
						  "cannot pass '%s' as argument %d: expected '%s'",
						  GetTypeKindPrettyName(expr->inferredType.kind),
						  (int)(i + 1),
						  GetTypeKindPrettyName(node->signature->params[i].kind));
				}
			}

			ResolveType(&node->signature->returnType, context, node);

			node->inferredType = node->signature->returnType;

			if (!IsRegisterSized(node->inferredType))
			{
				// TODO: is this alignment correct?
				context->symTable->stackSize = (int)align_forward(context->symTable->stackSize, 16);

				node->returnSlotOffset = ReserveSpace(context->symTable, node->inferredType);
			}
		} break;

		case NodeKind_AddressOf:
		{
			AddressOfNode *node = As<AddressOfNode>(baseNode);

			AnalyzeExpression(node->what, context);

			if (IsLValue(node->what))
			{
				node->inferredType.kind = TypeKind_Pointer;
				node->inferredType.pointee = &node->what->inferredType;
			}
			else
			{
				Error(context, node->what, "cannot take the address of this expression");
				Note(context, node->what,
					 "the operand of '&' must be a variable, a field, an array element or a dereference");
			}
		} break;

		case NodeKind_Deref:
		{
			DerefNode *node = As<DerefNode>(baseNode);

			AnalyzeExpression(node->what, context);

			if (node->what->inferredType.kind == TypeKind_Pointer)
			{
				node->inferredType = *node->what->inferredType.pointee;

				ResolveType(&node->inferredType, context, node);
			}
			else
			{
				Error(context, node->what, "cannot dereference '%s': it is not a pointer",
					  GetTypeKindPrettyName(node->what->inferredType.kind));
			}
		} break;

		case NodeKind_FieldAccess:
		{
			FieldAccessNode *node = As<FieldAccessNode>(baseNode);

			if (!node->expr)
			{
				// implicit enum
				// color: Color = .Red;
				
				if (expectedType.kind == TypeKind_Enum)
				{
					EnumeratorInfo *enumerator = FindEnumerator(expectedType.enumInfo, node->fieldName);
					if (enumerator)
					{
						Int64LiteralNode *newNode = ReinterpretNode<Int64LiteralNode>(node);
						newNode->inferredType = expectedType;
						newNode->value = enumerator->value;
					}
					else
					{
						Error(context, node, "enum " STR_FMT_QUOTED " has no enumerator " STR_FMT_QUOTED,
							  STR_ARG(expectedType.enumInfo->name),
							  STR_ARG(node->fieldName));
					}
				}
				else if (expectedType.kind == TypeKind_Unknown
						 || expectedType.kind == TypeKind_InferMe)
				{
					Error(context, node, "cannot infer the enum type of '." STR_FMT "' here",
						  STR_ARG(node->fieldName));
					Note(context, node,
						 "there is no expected enum type in this context, write the enum name explicitly");
				}
				else
				{
					Error(context, node, "cannot use '." STR_FMT "' here: expected type is '%s', not an enum",
						  STR_ARG(node->fieldName),
						  GetTypeKindPrettyName(expectedType.kind));
				}

				break;
			}

			if (node->expr->kind == NodeKind_Var)
			{
				Type *type = LookupType(context->typeTable, As<VarNode>(node->expr)->name);
				if (type && type->kind == TypeKind_Enum)
				{
					EnumeratorInfo *enumerator = FindEnumerator(type->enumInfo, node->fieldName);
					if (enumerator)
					{
						Int64LiteralNode *newNode = ReinterpretNode<Int64LiteralNode>(node);
						newNode->inferredType = *type;
						newNode->value = enumerator->value;
					}
					else
					{
						Error(context, node, "enum " STR_FMT_QUOTED " has no enumerator " STR_FMT_QUOTED,
							  STR_ARG(type->name),
							  STR_ARG(node->fieldName));
					}

					break;
				}
			}

			AnalyzeExpression(node->expr, context);

			auto HandleField = [&](Type *type)
			{
				if (type->kind == TypeKind_Struct)
				{
					StructField *field = FindField(type->structInfo, node->fieldName);
					if (field)
					{
						node->inferredType = field->type;
						node->fieldOffset = field->offset;
					}
					else
					{
						Error(context, node, "struct " STR_FMT_QUOTED " has no field " STR_FMT_QUOTED,
							  STR_ARG(type->structInfo->name),
							  STR_ARG(node->fieldName));
					}
				}
				else if (type->kind == TypeKind_Slice)
				{
					if (node->fieldName == "data")
					{
						node->inferredType.kind = TypeKind_Pointer;
						node->inferredType.pointee = type->arrayElementType;
						node->fieldOffset = 0;
					}
					else if (node->fieldName == "count")
					{
						node->inferredType.kind = TypeKind_Int64;
						node->fieldOffset = 8;
					}
					else
					{
						Error(context, node, "a slice has no field " STR_FMT_QUOTED, STR_ARG(node->fieldName));
					}
				}
				else if (type->kind == TypeKind_Array)
				{
					if (node->fieldName == "count")
					{
						int arrayLength = type->arrayLength;
						Assert(arrayLength != 0);

						Int64LiteralNode *newNode = ReinterpretNode<Int64LiteralNode>(node);
						newNode->inferredType.kind = TypeKind_Int64;
						newNode->value = arrayLength;
					}
					else
					{
						Error(context, node, "an array has no field " STR_FMT_QUOTED, STR_ARG(node->fieldName));
					}
				}
				else if (type->kind == TypeKind_DynamicArray)
				{
					if (node->fieldName == "data")
					{
						node->inferredType.kind = TypeKind_Pointer;
						node->inferredType.pointee = type->arrayElementType;
						node->fieldOffset = 0;
					}
					else if (node->fieldName == "count")
					{
						node->inferredType.kind = TypeKind_Int64;
						node->fieldOffset = 8;
					}
					else if (node->fieldName == "capacity")
					{
						node->inferredType.kind = TypeKind_Int64;
						node->fieldOffset = 16;
					}
					else
					{
						Error(context, node, "a dynamic array has no field " STR_FMT_QUOTED, STR_ARG(node->fieldName));
					}
				}
				else
				{
					Assert(false);
				}
			};

			if (node->expr->inferredType.kind == TypeKind_Struct
				|| node->expr->inferredType.kind == TypeKind_Slice
				|| node->expr->inferredType.kind == TypeKind_Array
				|| node->expr->inferredType.kind == TypeKind_DynamicArray)
			{
				HandleField(&node->expr->inferredType);
			}
			else if (node->expr->inferredType.kind == TypeKind_Pointer)
			{
				// auto dereference
				// pointer.field

				if (node->expr->inferredType.pointee->kind == TypeKind_Struct
					|| node->expr->inferredType.pointee->kind == TypeKind_Slice
					|| node->expr->inferredType.pointee->kind == TypeKind_Array
					|| node->expr->inferredType.pointee->kind == TypeKind_DynamicArray)
				{
					ResolveType(node->expr->inferredType.pointee, context, node->expr);

					HandleField(node->expr->inferredType.pointee);
				}
				else
				{
					Error(context, node->expr,
						  "cannot access field " STR_FMT_QUOTED ": it is not a pointer to a struct",
						  STR_ARG(node->fieldName));
				}
			}
			else
			{
				Error(context, node->expr, "cannot access field " STR_FMT_QUOTED " of '%s': it is not a struct",
					  STR_ARG(node->fieldName),
					  GetTypeKindPrettyName(node->expr->inferredType.kind));
			}
		} break;

		case NodeKind_ArrayIndexAccess:
		{
			ArrayIndexAccessNode *node = As<ArrayIndexAccessNode>(baseNode);

			AnalyzeExpression(node->arrayExpr, context);
			AnalyzeExpression(node->indexExpr, context);

			if (!IsSignedInteger(node->indexExpr->inferredType))
			{
				Error(context, node->indexExpr,
					  "array index must be a signed integer, but it is '%s'",
					  GetTypeKindPrettyName(node->indexExpr->inferredType.kind));
				break;
			}

			if (node->arrayExpr->inferredType.kind == TypeKind_Pointer)
			{
				node->inferredType = *node->arrayExpr->inferredType.pointee;
			}
			else if (node->arrayExpr->inferredType.kind == TypeKind_Array
					 || node->arrayExpr->inferredType.kind == TypeKind_Slice
					 || node->arrayExpr->inferredType.kind == TypeKind_DynamicArray)
			{
				node->inferredType = *node->arrayExpr->inferredType.arrayElementType;
			}
			else
			{
				Error(context, node->arrayExpr,
					  "cannot index '%s': it is not an array or a pointer",
					  GetTypeKindPrettyName(node->arrayExpr->inferredType.kind));
			}
		} break;

		case NodeKind_Cast:
		{
			CastNode *node = As<CastNode>(baseNode);

			AnalyzeExpression(node->what, context);

			if (IsInteger(node->targetType))
			{
				if (IsInteger(node->what->inferredType))
				{
					node->inferredType = node->targetType;
					break;
				}

				if (IsFloatingPoint(node->what->inferredType))
				{
					node->inferredType = node->targetType;
					break;
				}

				if (node->what->inferredType.kind == TypeKind_Enum)
				{
					node->inferredType = node->targetType;
					break;
				}

				if (node->what->inferredType.kind == TypeKind_Pointer)
				{
					node->inferredType = node->targetType;
					break;
				}
			}

			if (IsFloatingPoint(node->targetType))
			{
				if (IsInteger(node->what->inferredType))
				{
					node->inferredType = node->targetType;
					break;
				}

				if (IsFloatingPoint(node->what->inferredType))
				{
					node->inferredType = node->targetType;
					break;
				}
			}

			if (node->targetType.kind == TypeKind_Pointer)
			{
				if (node->what->inferredType.kind == TypeKind_Pointer)
				{
					// any pointer can be cast into any pointer
					node->inferredType = node->targetType;
					break;
				}
			}

			Error(context, node, "cannot cast '%s' to '%s'",
				  GetTypeKindPrettyName(node->what->inferredType.kind),
				  GetTypeKindPrettyName(node->targetType.kind));
		} break;

		case NodeKind_Break:
		{
			if (!(context->currentLoop
				  && (context->currentLoop->kind == NodeKind_While
					  || context->currentLoop->kind == NodeKind_For)))
			{
				Error(context, baseNode,
					  "'break' is only allowed in 'while' and 'for' loops");
			}
		} break;

		case NodeKind_Continue:
		{
			if (!(context->currentLoop
				  && (context->currentLoop->kind == NodeKind_While
					  || context->currentLoop->kind == NodeKind_For)))
			{
				Error(context, baseNode,
					  "'continue' is only allowed in 'while' and 'for' loops");
			}
		} break;

		case NodeKind_Defer:
		{
			DeferNode *node = As<DeferNode>(baseNode);

			AnalyzeStatement(node->what, context);
		} break;

		case NodeKind_Switch:
		{
			SwitchNode *node = As<SwitchNode>(baseNode);

			AnalyzeExpression(node->expr, context);

			if (IsInteger(node->expr->inferredType)
				|| node->expr->inferredType.kind == TypeKind_Enum)
			{
				for (CaseNode *caseNode : node->cases)
				{
					AnalyzeExpression(caseNode->label, context, node->expr->inferredType);

					AnalyzeStatement(caseNode->body, context);

					i64 result;
					if (TryEvaluateConstantExpression(caseNode->label, context, &result))
					{
						caseNode->labelValue = result;
					}
					else
					{
						Error(context, caseNode->label, "case label must be constant");
					}
				}

				if (node->defaultBody)
				{
					AnalyzeStatement(node->defaultBody, context);
				}
			}
			else
			{
				Error(context, node->expr, "switch expression must be an integer or an enum");
			}
		} break;

		case NodeKind_Proxy:
		{
			ProxyNode *node = As<ProxyNode>(baseNode);

			AnalyzeExpression(node->proxy, context, expectedType);

			node->inferredType = node->proxy->inferredType;
		} break;
	}
}

internal void
AnalyzeStatement(Node *baseNode,
				 SemanticContext *context)
{
	switch (baseNode->kind)
	{
		case NodeKind_VarDecl:
		{
			VarDeclNode *node = As<VarDeclNode>(baseNode);

			if (node->expr)
			{
				AnalyzeExpression(node->expr, context);

				if (node->type.kind == TypeKind_InferMe)
				{
					node->type = node->expr->inferredType;
				}
			}

			ResolveType(&node->type, context, node);

			if (node->type.kind != TypeKind_Void)
			{
				if (!LookupSymbol(context->symTable, node->name, context->symTable->scopeStart))
				{
					Symbol *symbol = DeclareSymbol(context->symTable, node->name, node->type);

					node->stackOffset = symbol->stackOffset;

					if (node->expr)
					{
						if (!CanImplicitlyCast(symbol->type, node->expr, context))
						{
							Error(context, node->expr,
								  "cannot initialize " STR_FMT_QUOTED " of type '%s' with a value of type '%s'",
								  STR_ARG(node->name),
								  GetTypeKindPrettyName(symbol->type.kind),
								  GetTypeKindPrettyName(node->expr->inferredType.kind));
						}
					}
				}
				else
				{
					Error(context, node, "variable " STR_FMT_QUOTED " is already declared in this scope",
						  STR_ARG(node->name));
				}
			}
			else
			{
				Error(context, node, "cannot declare variable " STR_FMT_QUOTED " of type 'void'",
					  STR_ARG(node->name));
			}
		} break;

		case NodeKind_Assign:
		{
			AssignNode *node = As<AssignNode>(baseNode);

			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context, node->lhs->inferredType);

			if (IsLValue(node->lhs))
			{
				if (!CanImplicitlyCast(node->lhs->inferredType, node->rhs, context))
				{
					Error(context, node->rhs, "cannot assign a value of type '%s' to '%s'",
						  GetTypeKindPrettyName(node->rhs->inferredType.kind),
						  GetTypeKindPrettyName(node->lhs->inferredType.kind));
				}
			}
			else
			{
				Error(context, node->lhs,
					  "cannot assign to this expression");
				Note(context, node->lhs,
					 "the left side of '=' must be a variable, a field, an array element or a dereference");
			}
		} break;

		case NodeKind_If:
		{
			IfNode *node = As<IfNode>(baseNode);

			if (node->elseBlock)
			{
				AnalyzeExpression(node->condition, context);
				AnalyzeStatement(node->thenBlock, context);
				AnalyzeStatement(node->elseBlock, context);
			}
			else
			{
				AnalyzeExpression(node->condition, context);
				AnalyzeStatement(node->thenBlock, context);
			}

			if (node->condition->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->condition,
					  "'if' condition must be of type 'bool', but it is '%s'",
					  GetTypeKindPrettyName(node->condition->inferredType.kind));
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);

			AnalyzeExpression(node->condition, context);

			Node *saveCurrentLoop = context->currentLoop;
			context->currentLoop = node;

			AnalyzeBlock(node->body, context);

			context->currentLoop = saveCurrentLoop;

			if (node->condition->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->condition,
					  "'while' condition must be of type 'bool', but it is '%s'",
					  GetTypeKindPrettyName(node->condition->inferredType.kind));
			}
		} break;

		case NodeKind_For:
		{
			ForNode *node = As<ForNode>(baseNode);

			Scope scope = EnterScope(context->symTable);

			AnalyzeStatement(node->init, context);
			AnalyzeExpression(node->cond, context);
			AnalyzeStatement(node->incr, context);

			Node *saveCurrentLoop = context->currentLoop;
			context->currentLoop = node;

			AnalyzeBlock(node->body, context);

			context->currentLoop = saveCurrentLoop;

			LeaveScope(context->symTable, scope);

			if (node->cond->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->cond,
					  "'for' condition must be of type 'bool', but it is '%s'",
					  GetTypeKindPrettyName(node->cond->inferredType.kind));
			}
		} break;

		case NodeKind_Print:
		{
			PrintNode *node = As<PrintNode>(baseNode);

			AnalyzeExpression(node->expr, context);
		} break;

		case NodeKind_Block:
		{
			AnalyzeBlock(baseNode, context);
		} break;

		case NodeKind_Return:
		{
			ReturnNode *node = As<ReturnNode>(baseNode);

			if (node->expr)
			{
				AnalyzeExpression(node->expr, context);
				
				if (!CanImplicitlyCast(context->currentFunction->returnType, node->expr, context))
				{
					Error(context, node,
						  "cannot return a value of type '%s' from " STR_FMT_QUOTED ": its return type is '%s'",
						  GetTypeKindPrettyName(node->expr->inferredType.kind),
						  STR_ARG(context->currentFunction->name),
						  GetTypeKindPrettyName(context->currentFunction->returnType.kind));
				}
			}
			else
			{
				// bare return;

				if (context->currentFunction->returnType.kind != TypeKind_Void)
				{
					Error(context, node, STR_FMT_QUOTED " must return a value of type '%s'",
						  STR_ARG(context->currentFunction->name),
						  GetTypeKindPrettyName(context->currentFunction->returnType.kind));
				}
			}
		} break;

		case NodeKind_Asm:
		{
			// do nothing
		} break;

		case NodeKind_Yield:
		{
			YieldNode *node = As<YieldNode>(baseNode);

			node->yieldIndex = ++context->currentFunction->yieldIndex;
		} break;

		default:
		{
			AnalyzeExpression(baseNode, context);
		} break;
	}
}

internal void
AnalyzeTopLevelStatement(Node *baseNode,
						 SemanticContext *context)
{
	switch (baseNode->kind)
	{
		case NodeKind_Func:
		{
			FuncNode *node = As<FuncNode>(baseNode);

			ResolveType(&node->returnType, context, node);

			Scope scope = EnterScope(context->symTable);

			if (!IsRegisterSized(node->returnType))
			{
				local_persist Type voidType = { TypeKind_Void };

				Type voidPtrType = {};
				voidPtrType.kind = TypeKind_Pointer;
				voidPtrType.pointee = &voidType;

				// declare the hidden struct pointer, which is the first argument
				int stackOffset = ReserveSpace(context->symTable, voidPtrType);

				// TODO: [rbp - 8] is hardcoded everywhere else
				Assert(stackOffset == 8);
			}

			for (ParamNode *param : node->params)
			{
				ResolveType(&param->type, context, param);

				if (!LookupSymbol(context->symTable, param->name, 0))
				{
					Symbol *symbol = DeclareSymbol(context->symTable, param->name, param->type);
					param->stackOffset = symbol->stackOffset;
				}
				else
				{
					Error(context, param, "parameter " STR_FMT_QUOTED " is already declared",
						  STR_ARG(param->name));
				}
			}

			FuncNode *saveCurrentFunction = context->currentFunction;
			context->currentFunction = node;

			if (node->body)
			{
				AnalyzeBlock(node->body, context);
			}
			else
			{
				// it's a foreign function
			}

			context->currentFunction = saveCurrentFunction;

			LeaveScope(context->symTable, scope);
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

internal void
EarlyAnalyze(Node *baseNode,
			 SemanticContext *context,
			 Arena *arena)
{
	switch (baseNode->kind)
	{
		default:
		{
			Assert(false);
		} break;

		case NodeKind_Func:
		{
			FuncNode *node = As<FuncNode>(baseNode);

			if (!LookupFunction(context->funcTable, node->name))
			{
				Function *func = DeclareFunction(context->funcTable, node->name);
				func->linkName = node->linkName;

				func->info = push_struct<ProcInfo>(arena);
				func->info->returnType = node->returnType;
				func->info->isVariadic = node->isVariadic;
				func->info->isForeign = node->isForeign;
				func->info->params = push_slice<Type>(arena, node->params.count);

				for (usize i = 0; i < node->params.count; i++)
				{
					func->info->params[i] = node->params[i]->type;
				}
			}
			else
			{
				Error(context, node, "redefinition of procedure " STR_FMT_QUOTED, STR_ARG(node->name));
			}
		} break;

		case NodeKind_StructDecl:
		{
			StructDeclNode *node = As<StructDeclNode>(baseNode);

			if (!LookupType(context->typeTable, node->name))
			{
				Type *type = DeclareType(context->typeTable, node->name);

				type->kind = TypeKind_Struct;
				type->structInfo = push_struct<StructInfo>(arena);
				type->structInfo->name = node->name;

				int offset = 0;
				int maxFieldAlignment = 0;

				for (StructFieldDeclNode *field : node->fields)
				{
					if (!FindField(type->structInfo, field->name))
					{
						ResolveType(&field->type, context, field);

						StructField fieldInfo = {};
						fieldInfo.name = field->name;
						fieldInfo.type = field->type;

						int size = SizeOfType(field->type);
						int alignment = AlignmentOfType(field->type);

						offset = (int)align_forward(offset, alignment);
						maxFieldAlignment = Max(maxFieldAlignment, alignment);

						fieldInfo.offset = offset;
						offset += size;

						array_add(&type->structInfo->fields, fieldInfo);
					}
					else
					{
						Error(context, field,
							  "struct " STR_FMT_QUOTED " already has a field " STR_FMT_QUOTED,
							  STR_ARG(node->name),
							  STR_ARG(field->name));
					}
				}

				type->structInfo->size = (int)align_forward(offset, maxFieldAlignment);
				type->structInfo->alignment = maxFieldAlignment;
			}
			else
			{
				Error(context, node,
					  "cannot declare struct " STR_FMT_QUOTED ": a type with this name already exists",
					  STR_ARG(node->name));
			}
		} break;

		case NodeKind_EnumDecl:
		{
			EnumDeclNode *node = As<EnumDeclNode>(baseNode);

			if (!LookupType(context->typeTable, node->name))
			{
				Type *type = DeclareType(context->typeTable, node->name);

				type->kind = TypeKind_Enum;
				type->enumInfo = push_struct<EnumInfo>(arena);
				type->enumInfo->name = node->name;

				i64 enumeratorValue = 0;

				for (EnumeratorDeclNode *enumerator : node->enumerators)
				{
					if (!FindEnumerator(type->enumInfo, enumerator->name))
					{
						EnumeratorInfo info = {};
						info.name = enumerator->name;
						info.value = enumeratorValue++;

						array_add(&type->enumInfo->enumerators, info);
					}
					else
					{
						Error(context, enumerator,
							  "enum " STR_FMT_QUOTED " already has an enumerator " STR_FMT_QUOTED,
							  STR_ARG(node->name),
							  STR_ARG(enumerator->name));
					}
				}
			}
			else
			{
				Error(context, node,
					  "cannot declare enum " STR_FMT_QUOTED ": a type with this name already exists",
					  STR_ARG(node->name));
			}
		} break;

		case NodeKind_ConstantDecl:
		{
			ConstantDeclNode *node = As<ConstantDeclNode>(baseNode);

			if (!LookupConstant(context->constTable, node->name))
			{
				Constant *constant = DeclareConstant(context->constTable, node->name);

				constant->value = EvaluateConstantExpression(node->expr, context);
			}
			else
			{
				Error(context, node, "redefinition of constant " STR_FMT_QUOTED, STR_ARG(node->name));
			}
		} break;

		case NodeKind_VarDecl:
		{
			VarDeclNode *node = As<VarDeclNode>(baseNode);

			if (!LookupSymbol(context->globalTable, node->name, 0))
			{
				ResolveType(&node->type, context, node);

				DeclareSymbol(context->globalTable, node->name, node->type);

				if (node->expr)
				{
					Error(context, node, "global variable " STR_FMT_QUOTED " cannot have an initializer",
						  STR_ARG(node->name));
				}
			}
			else
			{
				Error(context, node, "redefinition of global variable " STR_FMT_QUOTED, STR_ARG(node->name));
			}
		} break;

		case NodeKind_Asm:
		{
			// do nothing
		} break;

		case NodeKind_MacroDecl:
		{
			MacroDeclNode *node = As<MacroDeclNode>(baseNode);

			if (!LookupMacro(context->macroTable, node->name))
			{
				Macro *macro = DeclareMacro(context->macroTable, node->name);
				macro->decl = node;
			}
			else
			{
				Error(context, node, "redefinition of macro " STR_FMT_QUOTED, STR_ARG(node->name));
			}
		} break;
	}
}

void
SemanticPass(Node *_program,
			 SemanticContext *context,
			 Arena *arena)
{
	context->cstringLiterals = push_bump_array<GenerateCStringLiteral>(arena, 32);
	context->stringLiterals  = push_bump_array<GenerateStringLiteral>(arena, 32);

	context->funcTable   = push_struct<FunctionTable>(arena);
	context->symTable    = push_struct<SymbolTable>(arena);
	context->globalTable = push_struct<SymbolTable>(arena);
	context->typeTable   = push_struct<TypeTable>(arena);
	context->constTable  = push_struct<ConstantsTable>(arena);
	context->macroTable  = push_struct<MacroTable>(arena);

	BlockNode *program = As<BlockNode>(_program);
	for (Node *it : program->statements)
	{
		EarlyAnalyze(it, context, arena);
	}

	if (context->hadError)
	{
		return;
	}

	for (Node *it : program->statements)
	{
		if (it->kind == NodeKind_Func)
		{
			FuncNode *functionDef = As<FuncNode>(it);

			// clear the symbol table for every function
			memset(context->symTable, 0, sizeof(*context->symTable));

			AnalyzeTopLevelStatement(functionDef, context);

			if (functionDef->body)
			{
				BlockNode *functionBody = As<BlockNode>(functionDef->body);

				int stackSize = (context->symTable->maxStackSize + 15) & ~15;
				functionBody->stackSize = stackSize;
			}
		}
	}
}
