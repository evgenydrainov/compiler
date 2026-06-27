#include "semantic_pass.h"

#include "symbol_table.h"
#include "function_table.h"
#include "type_table.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

internal void
Error(SemanticContext *context,
	  Node *node,
	  const char *format,
	  ...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "line %d: ", node->line);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");

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
			return true;

		default:
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
		if (!type->structInfo)
		{
			Type *typeInfo = LookupType(context->typeTable, type->name);
			if (typeInfo)
			{
				type->structInfo = typeInfo->structInfo;
			}
			else
			{
				Error(context, nodeForError, STR_FMT_QUOTED ": unknown type", STR_ARG(type->name));

				local_persist StructInfo dummyStructInfo;
				type->structInfo = &dummyStructInfo;
			}
		}
		else
		{
			// type is already resolved
		}
	}
	//else if (type->kind == TypeKind_Pointer)
	//{
	//	ResolveType(type->pointerTo, context, nodeForError);
	//}
}

internal void
AnalyzeExpression(Node *baseNode,
				  SemanticContext *context);

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
		{
			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context);

			const char *opName = "";
			if (node->op == BinaryOp_Add)
			{
				opName = "add";
			}
			else if (node->op == BinaryOp_Subtract)
			{
				opName = "subtract";
			}
			else if (node->op == BinaryOp_Multiply)
			{
				opName = "multiply";
			}
			else if (node->op == BinaryOp_Divide)
			{
				opName = "divide";
			}
			else if (node->op == BinaryOp_Modulo)
			{
				opName = "divide";
			}

			if (TypesEqual(node->lhs->inferredType, node->rhs->inferredType))
			{
				if (node->lhs->inferredType.kind == TypeKind_Int64)
				{
					node->inferredType = node->lhs->inferredType;
				}
				else
				{
					Error(context, node, "cannot %s '%s' and '%s': types are not numeric",
						  opName,
						  GetTypeKindPrettyName(node->lhs->inferredType.kind),
						  GetTypeKindPrettyName(node->rhs->inferredType.kind));
				}
			}
			else
			{
				Error(context, node, "cannot %s '%s' and '%s': types are different",
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

			if (TypesEqual(node->lhs->inferredType, node->rhs->inferredType))
			{
				if (node->lhs->inferredType.kind == TypeKind_Int64)
				{
					node->inferredType.kind = TypeKind_Bool;
				}
				else
				{
					Error(context, node, "cannot compare '%s' and '%s': types are not numeric",
						  GetTypeKindPrettyName(node->lhs->inferredType.kind),
						  GetTypeKindPrettyName(node->rhs->inferredType.kind));
				}
			}
			else
			{
				Error(context, node, "cannot compare '%s' and '%s': types are different",
					  GetTypeKindPrettyName(node->lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->rhs->inferredType.kind));
			}
		} break;

		case BinaryOp_EqualEqual:
		case BinaryOp_NotEqual:
		{
			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context);

			if (TypesEqual(node->lhs->inferredType, node->rhs->inferredType))
			{
				node->inferredType.kind = TypeKind_Bool;
			}
			else
			{
				Error(context, node, "cannot compare '%s' and '%s': types are different",
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

internal bool
IsSignedInteger(Type type)
{
	return (type.kind == TypeKind_Int64
			|| type.kind == TypeKind_Int32
			|| type.kind == TypeKind_Int16
			|| type.kind == TypeKind_Int8);
}

internal void
AnalyzeExpression(Node *baseNode,
				  SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;
	FunctionTable *funcTable = context->funcTable;

	switch (baseNode->kind)
	{
		default:
		{
			Assert(false);
		} break;

		case NodeKind_Asm:
		{
			// do nothing
		} break;

		case NodeKind_Number:
		{
			baseNode->inferredType.kind = TypeKind_Int64;
		} break;

		//case NodeKind_String:
		//{
		//	baseNode->inferredType.kind = TypeKind_String;
		//} break;

		case NodeKind_CString:
		{
			baseNode->inferredType.kind = TypeKind_Pointer;

			local_persist Type int8Type = { TypeKind_Int8 };
			baseNode->inferredType.pointerTo = &int8Type;

			CStringNode *node = As<CStringNode>(baseNode);

			GenerateCStringLiteral literal = {};
			literal.value = node->value;
			literal.uniqueLabelId = node->uniqueId;

			ArrayAdd(&context->cstringLiterals, literal);
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

			if (node->op == UnaryOp_Negate)
			{
				if (IsSignedInteger(node->expr->inferredType))
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
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->name));
			}
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(baseNode);

			Function *function = LookupFunction(funcTable, node->name);
			if (function)
			{
				if (node->numExpressions == function->numParams)
				{
					for (int i = 0;
						 i < node->numExpressions;
						 i++)
					{
						Node *expr = node->expressions[i];

						AnalyzeExpression(expr, context);

						if (!TypesEqual(expr->inferredType, function->params[i].type))
						{
							Error(context, expr, "cannot pass argument of type '%s': function expects '%s'",
								  GetTypeKindPrettyName(expr->inferredType.kind),
								  GetTypeKindPrettyName(function->params[i].type.kind));
						}
					}

					node->inferredType = function->returnType;
				}
				else
				{
					Error(context, node, "cannot call " STR_FMT_QUOTED ": expected %d arguments, but got %d",
						  STR_ARG(function->name),
						  function->numParams,
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
	}
}

internal bool
CanImplicitCast(Type destType, Node *source)
{
	if (destType.kind == TypeKind_Unknown)
	{
		return false;
	}

	if (TypesEqual(destType, source->inferredType))
	{
		return true;
	}

	if (source->kind == NodeKind_Number)
	{
		NumberNode *number = As<NumberNode>(source);

		struct Range
		{
			i64 min;
			i64 max;
		};

		Range range = {};

		if (destType.kind == TypeKind_Int32)
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

		if (number->int64Value >= range.min && number->int64Value <= range.max)
		{
			return true;
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
					if (!CanImplicitCast(symbol->type, node->expr))
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
			AnalyzeExpression(node->rhs, context);

			if (IsLValue(node->lhs))
			{
				if (!CanImplicitCast(node->lhs->inferredType, node->rhs))
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
				Error(context, node->condition, "'if' condition must be bool");
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);

			AnalyzeExpression(node->condition, context);
			AnalyzeBlock(node->body, context);

			if (node->condition->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->condition, "'while' condition must be bool");
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
				
				if (!CanImplicitCast(context->currentFunction->returnType, node->expr))
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

				if (param->type.kind == TypeKind_Struct)
				{
					Error(context, param, "cannot pass struct by value");
				}

				ResolveType(&param->type, context, param);

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

			AnalyzeBlock(node->body, context);

			context->currentFunction = saveCurrentFunction;

			LeaveScope(symTable, scope);
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

void
SemanticPass(Node *_program,
			 SemanticContext *context,
			 Arena *arena)
{
	context->cstringLiterals = PushBumpArray<GenerateCStringLiteral>(arena, 32);

	FunctionTable *funcTable = PushStruct<FunctionTable>(arena);
	SymbolTable *symTable = PushStruct<SymbolTable>(arena);
	TypeTable *typeTable = PushStruct<TypeTable>(arena);

	context->funcTable = funcTable;
	context->symTable = symTable;
	context->typeTable = typeTable;

	BlockNode *program = As<BlockNode>(_program);

	for (Node *it : program->statements)
	{
		if (it->kind == NodeKind_Func)
		{
			FuncNode *functionDef = As<FuncNode>(it);

			if (!LookupFunction(funcTable, functionDef->name))
			{
				Function *func = DeclareFunction(funcTable, functionDef->name);
				func->numParams = functionDef->numParams;
				func->returnType = functionDef->returnType;

				for (int paramIndex = 0;
					 paramIndex < functionDef->numParams;
					 paramIndex++)
				{
					ParamNode *paramNode = As<ParamNode>(functionDef->params[paramIndex]);

					func->params[paramIndex].type = paramNode->type;
				}
			}
			else
			{
				Error(context, functionDef, "function " STR_FMT_QUOTED " was already defined", STR_ARG(functionDef->name));
			}
		}
		else if (it->kind == NodeKind_StructDecl)
		{
			StructDeclNode *node = As<StructDeclNode>(it);

			if (!LookupType(typeTable, node->name))
			{
				Type *type = DeclareType(typeTable, node->name);

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
		}
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
			if (!functionDef->isForeign)
			{
				BlockNode *functionBody = As<BlockNode>(functionDef->body);

				// clear the symbol table for every function
				memset(symTable, 0, sizeof(*symTable));

				AnalyzeTopLevelStatement(functionDef, context);

				int stackSize = (symTable->maxStackSize + 15) & ~15;
				functionBody->stackSize = stackSize;
			}
		}
	}
}
