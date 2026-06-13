#include "semantic_pass.h"
#include "symbol_table.h"
#include "function_table.h"
#include <stdio.h>
#include <stdarg.h>

internal void
Error(SemanticContext *context,
	  AstNode *node,
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
AnalyzeStatement(AstNode *node,
				 SemanticContext *context);

internal void
AnalyzeBlock(AstNode *node,
			 SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;

	Assert(node->type == NodeType_Block);

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
AnalyzeExpression(AstNode *node,
				  SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;
	FunctionTable *funcTable = context->funcTable;

	switch (node->type)
	{
		case NodeType_Number:
		{
			// do nothing
		} break;

		case NodeType_Add:
		case NodeType_Subtract:
		case NodeType_Multiply:
		case NodeType_Divide:
		case NodeType_Less:
		case NodeType_Greater:
		case NodeType_EqualEqual:
		case NodeType_LessEqual:
		case NodeType_GreaterEqual:
		case NodeType_NotEqual:
		{
			AnalyzeExpression(node->binary.lhs, context);
			AnalyzeExpression(node->binary.rhs, context);
		} break;

		case NodeType_Var:
		{
			Symbol *symbol = LookupSymbol(symTable, node->var.name, 0);
			if (symbol)
			{
				node->var.stackOffset = symbol->offset;
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->var.name));
			}
		} break;

		case NodeType_Call:
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
						AnalyzeExpression(node->call.expressions[i], context);
					}
				}
				else
				{
					Error(context, node, "expected %d arguments, but got %d", function->numParams, node->call.numExpressions);
				}
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": identifier not found", STR_ARG(node->call.name));
			}
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

internal void
AnalyzeStatement(AstNode *node,
				 SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;

	switch (node->type)
	{
		case NodeType_VarDecl:
		{
			AnalyzeExpression(node->assign.expr, context);

			if (LookupSymbol(symTable, node->assign.name, symTable->scopeStart))
			{
				Error(context, node, STR_FMT_QUOTED ": redefinition", STR_ARG(node->assign.name));
			}
			else
			{
				Symbol *symbol = DeclareSymbol(symTable, node->assign.name);
				node->assign.stackOffset = symbol->offset;
			}
		} break;

		case NodeType_Assign:
		{
			AnalyzeExpression(node->assign.expr, context);

			Symbol *symbol = LookupSymbol(symTable, node->assign.name, 0);
			if (symbol)
			{
				node->assign.stackOffset = symbol->offset;
			}
			else
			{
				Error(context, node, STR_FMT_QUOTED ": undeclared identifier", STR_ARG(node->assign.name));
			}
		} break;

		case NodeType_If:
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
		} break;

		case NodeType_While:
		{
			AnalyzeExpression(node->_while.condition, context);
			AnalyzeBlock(node->_while.body, context);
		} break;

		case NodeType_Print:
		{
			AnalyzeExpression(node->print.expr, context);
		} break;

		case NodeType_Block:
		{
			AnalyzeBlock(node, context);
		} break;

		case NodeType_Return:
		{
			AnalyzeExpression(node->ret.expr, context);
		} break;

		default:
		{
			AnalyzeExpression(node, context);
		} break;
	}
}

internal void
AnalyzeTopLevelStatement(AstNode *node,
						 SemanticContext *context)
{
	SymbolTable *symTable = context->symTable;

	switch (node->type)
	{
		case NodeType_Func:
		{
			Scope scope = EnterScope(symTable);

			for (int i = 0;
				 i < node->func.numParams;
				 i++)
			{
				Symbol *symbol = DeclareSymbol(symTable, node->func.params[i].name);
				node->func.params[i].stackOffset = symbol->offset;
			}

			AnalyzeBlock(node->func.body, context);

			LeaveScope(symTable, scope);
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

void
SemanticPass(AstNode *program,
			 SemanticContext *context)
{
	Assert(program->type == NodeType_Block);

	FunctionTable funcTable = {};
	context->funcTable = &funcTable;

	for (int i = 0;
		 i < program->block.numStatements;
		 i++)
	{
		Assert(program->block.statements[i]->type == NodeType_Func);

		AstNode *functionDef = program->block.statements[i];

		if (LookupFunction(&funcTable, functionDef->func.name))
		{
			Error(context, functionDef, "function " STR_FMT_QUOTED " already has a body", STR_ARG(functionDef->func.name));
		}
		else
		{
			Function *func = DeclareFunction(&funcTable, functionDef->func.name);
			func->numParams = functionDef->func.numParams;
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
		Assert(program->block.statements[i]->type == NodeType_Func);

		AstNode *functionDef = program->block.statements[i];
		AstNode *functionBody = functionDef->func.body;

		SymbolTable symTable = {};
		context->symTable = &symTable;

		AnalyzeTopLevelStatement(functionDef, context);

		int stackSize = (symTable.maxStackSize + 15) & ~15;
		functionBody->block.stackSize = stackSize;
	}
}
