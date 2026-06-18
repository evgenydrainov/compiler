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
AnalyzeBlock(Node *node,
			 SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;

	Assert(node->kind == NodeKind_Block);

	Scope scope = EnterScope(symTable);

	for (int i = 0;
		 i < node->block.numStatements;
		 i++)
	{
		AnalyzeStatement(node->block.statements[i], context);
	}

	LeaveScope(symTable, scope);
}

internal void
AnalyzeExpression(Node *node,
				  SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;
	FunctionTable *funcTable = context->funcTable;

	switch (node->kind)
	{
		case NodeKind_Number:
		{
			node->inferredType.kind = TypeKind_Int64;
		} break;

		case NodeKind_Bool:
		{
			node->inferredType.kind = TypeKind_Bool;
		} break;

		case NodeKind_Add:
		case NodeKind_Subtract:
		case NodeKind_Multiply:
		case NodeKind_Divide:
		{
			AnalyzeExpression(node->binary.lhs, context);
			AnalyzeExpression(node->binary.rhs, context);

			if (TypesEqual(node->binary.lhs->inferredType, node->binary.rhs->inferredType))
			{
				if (node->binary.lhs->inferredType.kind == TypeKind_Int64)
				{
					node->inferredType = node->binary.lhs->inferredType;
				}
				else
				{
					Error(context, node, "cannot %s '%s' and '%s': types are not numeric",
						  GetNodeKindPrettyName(node->kind),
						  GetTypeKindPrettyName(node->binary.lhs->inferredType.kind),
						  GetTypeKindPrettyName(node->binary.rhs->inferredType.kind));
				}
			}
			else
			{
				Error(context, node, "cannot %s '%s' and '%s': types are different",
					  GetNodeKindPrettyName(node->kind),
					  GetTypeKindPrettyName(node->binary.lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->binary.rhs->inferredType.kind));
			}
		} break;

		case NodeKind_Less:
		case NodeKind_Greater:
		case NodeKind_LessEqual:
		case NodeKind_GreaterEqual:
		{
			AnalyzeExpression(node->binary.lhs, context);
			AnalyzeExpression(node->binary.rhs, context);

			if (TypesEqual(node->binary.lhs->inferredType, node->binary.rhs->inferredType))
			{
				if (node->binary.lhs->inferredType.kind == TypeKind_Int64)
				{
					node->inferredType.kind = TypeKind_Bool;
				}
				else
				{
					Error(context, node, "cannot compare '%s' and '%s': types are not numeric",
						  GetTypeKindPrettyName(node->binary.lhs->inferredType.kind),
						  GetTypeKindPrettyName(node->binary.rhs->inferredType.kind));
				}
			}
			else
			{
				Error(context, node, "cannot compare '%s' and '%s': types are different",
					  GetTypeKindPrettyName(node->binary.lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->binary.rhs->inferredType.kind));
			}
		} break;

		case NodeKind_EqualEqual:
		case NodeKind_NotEqual:
		{
			AnalyzeExpression(node->binary.lhs, context);
			AnalyzeExpression(node->binary.rhs, context);

			if (TypesEqual(node->binary.lhs->inferredType, node->binary.rhs->inferredType))
			{
				node->inferredType.kind = TypeKind_Bool;
			}
			else
			{
				Error(context, node, "cannot compare '%s' and '%s': types are different",
					  GetTypeKindPrettyName(node->binary.lhs->inferredType.kind),
					  GetTypeKindPrettyName(node->binary.rhs->inferredType.kind));
			}
		} break;

		case NodeKind_Var:
		{
			Symbol *symbol = LookupSymbol(symTable, node->var.name, 0);
			if (symbol)
			{
				node->var.stackOffset = symbol->stackOffset;
				node->inferredType = symbol->type;
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->var.name));
			}
		} break;

		case NodeKind_Call:
		{
			Function *function = LookupFunction(funcTable, node->call.name);
			if (function)
			{
				if (node->call.numExpressions == function->numParams)
				{
					for (int i = 0;
						 i < node->call.numExpressions;
						 i++)
					{
						Node *expr = node->call.expressions[i];

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
						  node->call.numExpressions);
				}
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": identifier not found", STR_ARG(node->call.name));
			}
		} break;

		case NodeKind_AddressOf:
		{
			AnalyzeExpression(node->addressOf.what, context);

			node->inferredType.kind = TypeKind_Pointer;
			node->inferredType.pointerTo = &node->addressOf.what->inferredType;
		} break;

		case NodeKind_Deref:
		{
			AnalyzeExpression(node->deref.what, context);

			Assert(node->deref.what->inferredType.kind == TypeKind_Pointer);
			node->inferredType = *node->deref.what->inferredType.pointerTo;
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

internal void
AnalyzeStatement(Node *node,
				 SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;

	switch (node->kind)
	{
		case NodeKind_VarDecl:
		{
			if (node->varDecl.expr)
			{
				AnalyzeExpression(node->varDecl.expr, context);
			}

			if (!LookupSymbol(symTable, node->varDecl.name, symTable->scopeStart))
			{
				Symbol *symbol = DeclareSymbol(symTable, node->varDecl.name);
				symbol->type = node->varDecl.type;

				node->varDecl.stackOffset = symbol->stackOffset;

				if (node->varDecl.expr)
				{
					if (!TypesEqual(node->varDecl.expr->inferredType, symbol->type))
					{
						Error(context, node->varDecl.expr, "cannot assign '%s' to '%s'",
							  GetTypeKindPrettyName(node->varDecl.expr->inferredType.kind),
							  GetTypeKindPrettyName(symbol->type.kind));
					}
				}
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": redefinition", STR_ARG(node->varDecl.name));
			}
		} break;

		case NodeKind_Assign:
		{
			AnalyzeExpression(node->assign.expr, context);

			Symbol *symbol = LookupSymbol(symTable, node->assign.name, 0);
			if (symbol)
			{
				node->assign.stackOffset = symbol->stackOffset;

				if (!TypesEqual(node->assign.expr->inferredType, symbol->type))
				{
					Error(context, node->assign.expr, "cannot assign '%s' to '%s'",
						  GetTypeKindPrettyName(node->assign.expr->inferredType.kind),
						  GetTypeKindPrettyName(symbol->type.kind));
				}
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->assign.name));
			}
		} break;

		case NodeKind_If:
		{
			if (node->_if.elseBlock)
			{
				AnalyzeExpression(node->_if.condition, context);
				AnalyzeBlock(node->_if.thenBlock, context);
				AnalyzeBlock(node->_if.elseBlock, context);
			}
			else
			{
				AnalyzeExpression(node->_if.condition, context);
				AnalyzeBlock(node->_if.thenBlock, context);
			}

			if (node->_if.condition->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->_if.condition, "'if' condition must be bool");
			}
		} break;

		case NodeKind_While:
		{
			AnalyzeExpression(node->_while.condition, context);
			AnalyzeBlock(node->_while.body, context);

			if (node->_while.condition->inferredType.kind != TypeKind_Bool)
			{
				Error(context, node->_while.condition, "'while' condition must be bool");
			}
		} break;

		case NodeKind_Print:
		{
			AnalyzeExpression(node->print.expr, context);
		} break;

		case NodeKind_Block:
		{
			AnalyzeBlock(node, context);
		} break;

		case NodeKind_Return:
		{
			Type returnExpressionType = {};
			returnExpressionType.kind = TypeKind_Void;

			if (node->ret.expr)
			{
				AnalyzeExpression(node->ret.expr, context);
				
				returnExpressionType = node->ret.expr->inferredType;
			}

			if (!TypesEqual(returnExpressionType, context->currentFunction->func.returnType))
			{
				Error(context, node, "cannot return '%s': function return type is '%s'",
					  GetTypeKindPrettyName(returnExpressionType.kind),
					  GetTypeKindPrettyName(context->currentFunction->func.returnType.kind));
			}
		} break;

		default:
		{
			AnalyzeExpression(node, context);
		} break;
	}
}

internal void
AnalyzeTopLevelStatement(Node *node,
						 SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;

	switch (node->kind)
	{
		case NodeKind_Func:
		{
			Scope scope = EnterScope(symTable);

			for (int i = 0;
				 i < node->func.numParams;
				 i++)
			{
				Node *param = node->func.params[i];

				if (LookupSymbol(symTable, param->param.name, 0))
				{
					Error(context, param, STR_FMT_QUOTED ": redefinition", STR_ARG(param->param.name));
				}
				else
				{
					Symbol *symbol = DeclareSymbol(symTable, param->param.name);
					symbol->type = param->param.type;

					param->param.stackOffset = symbol->stackOffset;
				}
			}

			Node *saveCurrentFunction = context->currentFunction;
			context->currentFunction = node;

			AnalyzeBlock(node->func.body, context);

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
SemanticPass(Node *program,
			 SemanticContext *context,
			 Arena *arena)
{
	Assert(program->kind == NodeKind_Block);

	FunctionTable *funcTable = PushStruct(arena, FunctionTable);
	memset(funcTable, 0, sizeof(*funcTable));

	SymbolTable *symTable = PushStruct(arena, SymbolTable);
	memset(symTable, 0, sizeof(*symTable));

	context->funcTable = funcTable;

	for (int i = 0;
		 i < program->block.numStatements;
		 i++)
	{
		Assert(program->block.statements[i]->kind == NodeKind_Func);

		Node *functionDef = program->block.statements[i];

		if (!LookupFunction(funcTable, functionDef->func.name))
		{
			Function *func = DeclareFunction(funcTable, functionDef->func.name);
			func->numParams = functionDef->func.numParams;
			func->returnType = functionDef->func.returnType;

			for (int paramIndex = 0;
				 paramIndex < functionDef->func.numParams;
				 paramIndex++)
			{
				Node *paramNode = functionDef->func.params[paramIndex];

				func->params[paramIndex].type = paramNode->param.type;
			}
		}
		else
		{
			Error(context, functionDef, "function " STR_FMT_QUOTED " was already defined", STR_ARG(functionDef->func.name));
		}
	}

	if (context->hadError)
	{
		return;
	}

	for (int i = 0;
		 i < program->block.numStatements;
		 i++)
	{
		Assert(program->block.statements[i]->kind == NodeKind_Func);

		Node *functionDef = program->block.statements[i];
		Node *functionBody = functionDef->func.body;

		// clear the symbol table for every function
		memset(symTable, 0, sizeof(*symTable));
		context->symTable = symTable;

		AnalyzeTopLevelStatement(functionDef, context);

		int stackSize = (symTable->maxStackSize + 15) & ~15;
		functionBody->block.stackSize = stackSize;
	}
}
