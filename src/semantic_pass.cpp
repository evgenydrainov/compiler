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
		Assert(!type->structInfo);

		Type *typeInfo = LookupType(context->typeTable, type->name);
		if (typeInfo)
		{
			type->structInfo = typeInfo->structInfo;
		}
		else
		{
			Error(context, nodeForError, STR_FMT_QUOTED ": unknown type", STR_ARG(type->name));
		}
	}
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

		case NodeKind_Number:
		{
			baseNode->inferredType.kind = TypeKind_Int64;
		} break;

		case NodeKind_Bool:
		{
			baseNode->inferredType.kind = TypeKind_Bool;
		} break;

		case NodeKind_Binary:
		{
			AnalyzeBinaryExpression(baseNode, context);
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
			else
			{
				Error(context, node->expr, "cannot access field of non-struct");
			}
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
			}

			ResolveType(&node->type, context, node);

			if (!LookupSymbol(symTable, node->name, symTable->scopeStart))
			{
				Symbol *symbol = DeclareSymbol(symTable, node->name, node->type);

				node->stackOffset = symbol->stackOffset;

				if (node->expr)
				{
					if (!TypesEqual(node->expr->inferredType, symbol->type))
					{
						Error(context, node->expr, "cannot assign '%s' to '%s'",
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
				if (!TypesEqual(node->lhs->inferredType, node->rhs->inferredType))
				{
					Error(context, node->rhs, "cannot assign '%s' to '%s'",
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
				AnalyzeBlock(node->thenBlock, context);
				AnalyzeBlock(node->elseBlock, context);
			}
			else
			{
				AnalyzeExpression(node->condition, context);
				AnalyzeBlock(node->thenBlock, context);
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

			Type returnExpressionType = {};
			returnExpressionType.kind = TypeKind_Void;

			if (node->expr)
			{
				AnalyzeExpression(node->expr, context);
				
				returnExpressionType = node->expr->inferredType;
			}

			if (!TypesEqual(returnExpressionType, context->currentFunction->returnType))
			{
				Error(context, node, "cannot return '%s': function return type is '%s'",
					  GetTypeKindPrettyName(returnExpressionType.kind),
					  GetTypeKindPrettyName(context->currentFunction->returnType.kind));
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
			BlockNode *functionBody = As<BlockNode>(functionDef->body);

			// clear the symbol table for every function
			memset(symTable, 0, sizeof(*symTable));

			AnalyzeTopLevelStatement(functionDef, context);

			int stackSize = (symTable->maxStackSize + 15) & ~15;
			functionBody->stackSize = stackSize;
		}
	}
}
