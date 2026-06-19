#include "semantic_pass.h"
#include "symbol_table.h"
#include "function_table.h"
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

	for (int i = 0;
		 i < node->numStatements;
		 i++)
	{
		AnalyzeStatement(node->statements[i], context);
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
			return true;

		default:
			return false;
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
		case NodeKind_Number:
		{
			baseNode->inferredType.kind = TypeKind_Int64;
		} break;

		case NodeKind_Bool:
		{
			baseNode->inferredType.kind = TypeKind_Bool;
		} break;

		case NodeKind_Add:
		case NodeKind_Subtract:
		case NodeKind_Multiply:
		case NodeKind_Divide:
		{
			BinaryNode *node = As<BinaryNode>(baseNode);

			AnalyzeExpression(node->lhs, context);
			AnalyzeExpression(node->rhs, context);

			if (TypesEqual(node->lhs->inferredType, node->rhs->inferredType))
			{
				if (node->lhs->inferredType.kind == TypeKind_Int64)
				{
					node->inferredType = node->lhs->inferredType;
				}
				else
				{
					Error(context, node, "cannot %s '%s' and '%s': types are not numeric",
						  GetNodeKindPrettyName(node->kind),
						  GetTypeKindPrettyName(node->lhs->inferredType.kind),
						  GetTypeKindPrettyName(node->rhs->inferredType.kind));
				}
			}
			else
			{
				Error(context, node, "cannot %s '%s' and '%s': types are different",
					  GetNodeKindPrettyName(node->kind),
					  GetTypeKindPrettyName(node->lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->rhs->inferredType.kind));
			}
		} break;

		case NodeKind_Less:
		case NodeKind_Greater:
		case NodeKind_LessEqual:
		case NodeKind_GreaterEqual:
		{
			BinaryNode *node = As<BinaryNode>(baseNode);

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

		case NodeKind_EqualEqual:
		case NodeKind_NotEqual:
		{
			BinaryNode *node = As<BinaryNode>(baseNode);

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
			}
			else
			{
				Error(context, node->what, "cannot dereference non-pointer");
			}
		} break;

		default:
		{
			Assert(false);
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

			if (!LookupSymbol(symTable, node->name, symTable->scopeStart))
			{
				Symbol *symbol = DeclareSymbol(symTable, node->name);
				symbol->type = node->type;

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

			Scope scope = EnterScope(symTable);

			for (int i = 0;
				 i < node->numParams;
				 i++)
			{
				ParamNode *param = As<ParamNode>(node->params[i]);

				if (LookupSymbol(symTable, param->name, 0))
				{
					Error(context, param, STR_FMT_QUOTED ": redefinition", STR_ARG(param->name));
				}
				else
				{
					Symbol *symbol = DeclareSymbol(symTable, param->name);
					symbol->type = param->type;

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
	memset(funcTable, 0, sizeof(*funcTable));

	SymbolTable *symTable = PushStruct<SymbolTable>(arena);
	memset(symTable, 0, sizeof(*symTable));

	context->funcTable = funcTable;

	BlockNode *program = As<BlockNode>(_program);

	for (int i = 0;
		 i < program->numStatements;
		 i++)
	{
		Assert(program->statements[i]->kind == NodeKind_Func);

		FuncNode *functionDef = As<FuncNode>(program->statements[i]);

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

	if (context->hadError)
	{
		return;
	}

	for (int i = 0;
		 i < program->numStatements;
		 i++)
	{
		FuncNode *functionDef = As<FuncNode>(program->statements[i]);
		BlockNode *functionBody = As<BlockNode>(functionDef->body);

		// clear the symbol table for every function
		memset(symTable, 0, sizeof(*symTable));
		context->symTable = symTable;

		AnalyzeTopLevelStatement(functionDef, context);

		int stackSize = (symTable->maxStackSize + 15) & ~15;
		functionBody->stackSize = stackSize;
	}
}
