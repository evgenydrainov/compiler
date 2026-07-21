#include "semantic_pass.h"

#include "symbol_table.h"
#include "function_table.h"
#include "type_table.h"
#include "constants_table.h"

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
		fprintf(stderr, STR_FMT "(%d, %d): ",
				STR_ARG(node->location.fileName), node->location.line, node->location.column);
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
	}

	va_end(args);
	
	context->hadError = true;
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
	SymbolTable *symTable = context->symTable;

	BlockNode *node = As<BlockNode>(baseNode);

	Scope scope = EnterScope(symTable);

	for (Node *it : node->statements)
	{
		AnalyzeStatement(it, context);
	}

	LeaveScope(symTable, scope);
}

internal bool
IsLValue(Node *node)
{
	switch (node->kind)
	{
		case NodeKind_Var:
		case NodeKind_Deref:
		case NodeKind_FieldAccess:
		case NodeKind_ArrayIndexAccess:
			return true;

		default:
			return false;
	}
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
				Error(context, baseNode, "division by zero");
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
			Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->name));
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
		Error(context, baseNode, "not a constant expression");
	}

	return 0;
}

internal bool
TryEvaluateConstantExpression(Node *baseNode,
							  SemanticContext *context,
							  i64 *outResult)
{
	bool suppressErrors = context->suppressErrors;
	context->suppressErrors = true;

	i64 result = EvaluateConstantExpression(baseNode, context);
	context->suppressErrors = suppressErrors;

	if (!context->hadError)
	{
		*outResult = result;
		return true;
	}
	else
	{
		context->hadError = false;

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
				Error(context, nodeForError, STR_FMT_QUOTED ": unknown type", STR_ARG(type->name));

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
			if (arrayLength != 0)
			{
				type->arrayLength = (int)arrayLength;
			}
			else
			{
				Error(context, nodeForError, "array length cannot be zero");

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
		ResolveType(type->pointerTo, context, nodeForError);
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

			if (CanImplicitlyCast(node->lhs->inferredType, node->rhs, context))
			{
				node->inferredType = node->lhs->inferredType;
			}
			else
			{
				Error(context, node, "cannot %s", opName);
			}
		} break;

		case BinaryOp_Less:
		case BinaryOp_Greater:
		case BinaryOp_LessEqual:
		case BinaryOp_GreaterEqual:
		{
			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context);

			if (CanImplicitlyCast(node->lhs->inferredType, node->rhs, context))
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

			if (CanImplicitlyCast(node->lhs->inferredType, node->rhs, context))
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
				Error(context, node, "lhs and rhs must be boolean");
			}
		} break;
	}
}

void
AnalyzeExpression(Node *baseNode,
				  SemanticContext *context,
				  Type expectedType)
{
	SymbolTable    *symTable   = context->symTable;
	FunctionTable  *funcTable  = context->funcTable;
	TypeTable      *typeTable  = context->typeTable;
	ConstantsTable *constTable = context->constTable;

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
			baseNode->inferredType.pointerTo = &uint8Type;

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
					Error(context, node->expr, "cannot negate '%s'",
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
					Error(context, node->expr, "cannot negate '%s'",
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

			Symbol *symbol = LookupSymbol(symTable, node->name, 0);
			if (symbol)
			{
				node->stackOffset = symbol->stackOffset;
				node->inferredType = symbol->type;

				break;
			}

			Constant *constant = LookupConstant(constTable, node->name);
			if (constant)
			{
				static_assert(sizeof(Int64LiteralNode) <= sizeof(VarNode));

				SourceLocation location = node->location;

				Int64LiteralNode *newNode = (Int64LiteralNode *)node;
				*newNode = {};
				newNode->kind = NodeKind_Int64Literal;
				newNode->location = location;
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

			Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->name));
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(baseNode);

			Function *function = LookupFunction(funcTable, node->name);
			if (function)
			{
				bool good = (node->numExpressions == function->params.count);

				if (function->isVariadic)
				{
					good = (node->numExpressions >= function->params.count);
				}

				if (good)
				{
					for (int i = 0;
						 i < node->numExpressions;
						 i++)
					{
						Node *expr = node->expressions[i];

						AnalyzeExpression(expr, context);
					}

					for (usize i = 0;
						 i < function->params.count;
						 i++)
					{
						Node *expr = node->expressions[i];

						if (!CanImplicitlyCast(function->params[i].type, expr, context))
						{
							Error(context, expr, "cannot implicitly cast '%s' to '%s'",
								  GetTypeKindPrettyName(expr->inferredType.kind),
								  GetTypeKindPrettyName(function->params[i].type.kind));
						}
					}

					node->inferredType = function->returnType;
					node->linkName = function->linkName;
				}
				else
				{
					Error(context, node, "cannot call " STR_FMT_QUOTED ": expected %d arguments, but got %d",
						  STR_ARG(function->name),
						  function->params.count,
						  node->numExpressions);
				}
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": identifier not found", STR_ARG(node->name));
			}
		} break;

		case NodeKind_AddressOf:
		{
			AddressOfNode *node = As<AddressOfNode>(baseNode);

			AnalyzeExpression(node->what, context);

			if (IsLValue(node->what))
			{
				node->inferredType.kind = TypeKind_Pointer;
				node->inferredType.pointerTo = &node->what->inferredType;
			}
			else
			{
				Error(context, node->what, "cannot take address of non-lvalue");
			}
		} break;

		case NodeKind_Deref:
		{
			DerefNode *node = As<DerefNode>(baseNode);

			AnalyzeExpression(node->what, context);

			if (node->what->inferredType.kind == TypeKind_Pointer)
			{
				node->inferredType = *node->what->inferredType.pointerTo;

				ResolveType(&node->inferredType, context, node);
			}
			else
			{
				Error(context, node->what, "cannot dereference non-pointer");
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
						static_assert(sizeof(Int64LiteralNode) <= sizeof(FieldAccessNode));

						SourceLocation location = node->location;

						Int64LiteralNode *newNode = (Int64LiteralNode *)node;
						*newNode = {};
						newNode->kind = NodeKind_Int64Literal;
						newNode->location = location;
						newNode->inferredType = expectedType;
						newNode->value = enumerator->value;
					}
					else
					{
						Error(context, node, "enumerator " STR_FMT_QUOTED " does not exist",
							  STR_ARG(node->fieldName));
					}
				}
				else
				{
					Error(context, node, "");
				}

				break;
			}

			if (node->expr->kind == NodeKind_Var)
			{
				Type *type = LookupType(typeTable, As<VarNode>(node->expr)->name);
				if (type && type->kind == TypeKind_Enum)
				{
					EnumeratorInfo *enumerator = FindEnumerator(type->enumInfo, node->fieldName);
					if (enumerator)
					{
						static_assert(sizeof(Int64LiteralNode) <= sizeof(FieldAccessNode));

						SourceLocation location = node->location;

						Int64LiteralNode *newNode = (Int64LiteralNode *)node;
						*newNode = {};
						newNode->kind = NodeKind_Int64Literal;
						newNode->location = location;
						newNode->inferredType = *type;
						newNode->value = enumerator->value;
					}
					else
					{
						Error(context, node, "enumerator " STR_FMT_QUOTED " does not exist",
							  STR_ARG(node->fieldName));
					}

					break;
				}
			}

			AnalyzeExpression(node->expr, context);

			if (node->expr->inferredType.kind == TypeKind_Struct)
			{
				StructField *field = FindField(node->expr->inferredType.structInfo, node->fieldName);
				if (field)
				{
					node->inferredType = field->type;
					node->fieldOffset = field->offset;
				}
				else
				{
					Error(context, node, "struct has no field " STR_FMT_QUOTED,
						  STR_ARG(node->fieldName));
				}
			}
			else if (node->expr->inferredType.kind == TypeKind_Pointer)
			{
				// auto dereference
				// pointer.field

				if (node->expr->inferredType.pointerTo->kind == TypeKind_Struct)
				{
					ResolveType(node->expr->inferredType.pointerTo, context, node->expr);

					StructField *field = FindField(node->expr->inferredType.pointerTo->structInfo, node->fieldName);
					if (field)
					{
						node->inferredType = field->type;
						node->fieldOffset = field->offset;
					}
					else
					{
						Error(context, node, "struct has no field " STR_FMT_QUOTED,
							  STR_ARG(node->fieldName));
					}
				}
				else
				{
					Error(context, node->expr, "cannot access field of pointer to non-struct");
				}
			}
			else
			{
				Error(context, node->expr, "cannot access field of non-struct");
			}
		} break;

		case NodeKind_ArrayIndexAccess:
		{
			ArrayIndexAccessNode *node = As<ArrayIndexAccessNode>(baseNode);

			AnalyzeExpression(node->arrayExpr, context);
			AnalyzeExpression(node->indexExpr, context);

			if (!IsSignedInteger(node->indexExpr->inferredType))
			{
				Error(context, node->indexExpr, "");
				break;
			}

			if (node->arrayExpr->inferredType.kind == TypeKind_Pointer)
			{
				node->inferredType = *node->arrayExpr->inferredType.pointerTo;
			}
			else if (node->arrayExpr->inferredType.kind == TypeKind_Array)
			{
				node->inferredType = *node->arrayExpr->inferredType.arrayElementType;
			}
			else
			{
				Error(context, node->arrayExpr, "");
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

			Error(context, node, "cannot cast");
		} break;

		case NodeKind_Break:
		{
			// TODO
		} break;

		case NodeKind_Continue:
		{
			// TODO
		} break;
	}
}

internal void
AnalyzeStatement(Node *baseNode,
				 SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;

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

			if (!LookupSymbol(symTable, node->name, symTable->scopeStart))
			{
				Symbol *symbol = DeclareSymbol(symTable, node->name, node->type);

				node->stackOffset = symbol->stackOffset;

				if (node->expr)
				{
					if (!CanImplicitlyCast(symbol->type, node->expr, context))
					{
						Error(context, node->expr, "cannot implicitly cast '%s' to '%s'",
							  GetTypeKindPrettyName(node->expr->inferredType.kind),
							  GetTypeKindPrettyName(symbol->type.kind));
					}
				}
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": redefinition", STR_ARG(node->name));
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
					Error(context, node->rhs, "cannot implicitly cast '%s' to '%s'",
						  GetTypeKindPrettyName(node->rhs->inferredType.kind),
						  GetTypeKindPrettyName(node->lhs->inferredType.kind));
				}
			}
			else
			{
				Error(context, node->lhs, "cannot assign to non-lvalue");
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
					  "'if' condition must be boolean, it is of type '%s'",
					  GetTypeKindPrettyName(node->condition->inferredType.kind));
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);

			AnalyzeExpression(node->condition, context);
			AnalyzeBlock(node->body, context);

			if (node->condition->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->condition,
					  "'while' condition must be boolean, it is of type '%s'",
					  GetTypeKindPrettyName(node->condition->inferredType.kind));
			}
		} break;

		case NodeKind_For:
		{
			ForNode *node = As<ForNode>(baseNode);

			Scope scope = EnterScope(symTable);

			AnalyzeStatement(node->init, context);
			AnalyzeExpression(node->cond, context);
			AnalyzeStatement(node->incr, context);

			AnalyzeBlock(node->body, context);

			LeaveScope(symTable, scope);

			if (node->cond->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->cond,
					  "'for' condition must be boolean, it is of type '%s'",
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
					Error(context, node, "cannot implicitly cast '%s' to '%s'",
						  GetTypeKindPrettyName(node->expr->inferredType.kind),
						  GetTypeKindPrettyName(context->currentFunction->returnType.kind));
				}	
			}
			else
			{
				// bare return;

				if (context->currentFunction->returnType.kind != TypeKind_Void)
				{
					Error(context, node, "cannot return 'void': function return type is '%s'",
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
	SymbolTable *symTable = context->symTable;

	switch (baseNode->kind)
	{
		case NodeKind_Func:
		{
			FuncNode *node = As<FuncNode>(baseNode);

			if (node->returnType.kind == TypeKind_Struct)
			{
				Error(context, node, "cannot return struct by value");
			}

			Scope scope = EnterScope(symTable);

			for (int i = 0;
				 i < node->numParams;
				 i++)
			{
				ParamNode *param = As<ParamNode>(node->params[i]);

				ResolveType(&param->type, context, param);

				int size = SizeOfType(param->type);
				if (!(size == 1
					  || size == 2
					  || size == 4
					  || size == 8))
				{
					Error(context, param, "cannot pass argument of size %d by value", size);
				}

				if (LookupSymbol(symTable, param->name, 0))
				{
					Error(context, param, STR_FMT_QUOTED ": redefinition", STR_ARG(param->name));
				}
				else
				{
					Symbol *symbol = DeclareSymbol(symTable, param->name, param->type);

					param->stackOffset = symbol->stackOffset;
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

			LeaveScope(symTable, scope);
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
				func->returnType = node->returnType;
				func->linkName = node->name;
				func->isVariadic = node->isVariadic;

				if (node->isForeign)
				{
					if (node->foreignLinkName.count > 0)
					{
						func->linkName = node->foreignLinkName;
					}
				}

				for (int paramIndex = 0;
					 paramIndex < node->numParams;
					 paramIndex++)
				{
					ParamNode *paramNode = As<ParamNode>(node->params[paramIndex]);

					Parameter parameter = {};
					parameter.type = paramNode->type;

					array_add(&func->params, parameter);
				}
			}
			else
			{
				Error(context, node, "function " STR_FMT_QUOTED " was already defined", STR_ARG(node->name));
			}
		} break;

		case NodeKind_StructDecl:
		{
			StructDeclNode *node = As<StructDeclNode>(baseNode);

			if (!LookupType(context->typeTable, node->name))
			{
				Type *type = DeclareType(context->typeTable, node->name);

				type->kind = TypeKind_Struct;
				type->structInfo = PushStruct<StructInfo>(arena);
				type->structInfo->name = node->name;

				int offset = 0;
				for (StructFieldDeclNode *field : node->fields)
				{
					if (!FindField(type->structInfo, field->name))
					{
						ResolveType(&field->type, context, field);

						type->structInfo->fields[type->structInfo->numFields].name = field->name;
						type->structInfo->fields[type->structInfo->numFields].type = field->type;

						type->structInfo->fields[type->structInfo->numFields].offset = offset;
						offset += SizeOfType(field->type);

						type->structInfo->numFields++;
					}
					else
					{
						Error(context, field, STR_FMT_QUOTED ": redefinition", STR_ARG(field->name));
					}
				}

				type->structInfo->size = offset;
			}
			else
			{
				Error(context, node, "struct " STR_FMT_QUOTED " was already defined", STR_ARG(node->name));
			}
		} break;

		case NodeKind_EnumDecl:
		{
			EnumDeclNode *node = As<EnumDeclNode>(baseNode);

			if (!LookupType(context->typeTable, node->name))
			{
				Type *type = DeclareType(context->typeTable, node->name);

				type->kind = TypeKind_Enum;
				type->enumInfo = PushStruct<EnumInfo>(arena);
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
						Error(context, enumerator, STR_FMT_QUOTED ": redefinition", STR_ARG(enumerator->name));
					}
				}
			}
			else
			{
				Error(context, node, "enum " STR_FMT_QUOTED " was already defined", STR_ARG(node->name));
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
				Error(context, node, STR_FMT_QUOTED ": redefinition", STR_ARG(node->name));
			}
		} break;

		case NodeKind_VarDecl:
		{
			VarDeclNode *node = As<VarDeclNode>(baseNode);

			if (!LookupSymbol(context->globalTable, node->name, 0))
			{
				DeclareSymbol(context->globalTable, node->name, node->type);

				if (node->expr)
				{
					Error(context, node, "global variable assignment not allowed");
				}
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": redefinition", STR_ARG(node->name));
			}
		} break;
	}
}

void
SemanticPass(Node *_program,
			 SemanticContext *context,
			 Arena *arena)
{
	context->cstringLiterals = PushBumpArray<GenerateCStringLiteral>(arena, 32);
	context->stringLiterals  = PushBumpArray<GenerateStringLiteral>(arena, 32);

	context->funcTable   = PushStruct<FunctionTable>(arena);
	context->symTable    = PushStruct<SymbolTable>(arena);
	context->globalTable = PushStruct<SymbolTable>(arena);
	context->typeTable   = PushStruct<TypeTable>(arena);
	context->constTable  = PushStruct<ConstantsTable>(arena);

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
