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

internal void
Analyze(AstNode *node,
		SemanticContext *context)
{
	if (!node)
	{
		return;
	}

	SymbolTable *symTable = context->symTable;
	FunctionTable *funcTable = context->funcTable;

	switch (node->type)
	{
		case NodeType_Block:
		{
			int numSymbols = symTable->count;
			int stackSize = symTable->stackSize;
			int scopeStart = symTable->scopeStart;

			symTable->scopeStart = symTable->count;

			for (int i = 0;
				 i < node->block.numStatements;
				 i++)
			{
				Analyze(node->block.statements[i], context);
			}

			symTable->count = numSymbols;
			symTable->stackSize = stackSize;
			symTable->scopeStart = scopeStart;
		} break;

		case NodeType_VarDecl:
		{
			Analyze(node->assign.expr, context);

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
			Analyze(node->assign.expr, context);

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

		case NodeType_If:
		{
			Analyze(node->_if.condition, context);
			Analyze(node->_if.thenBlock, context);
			Analyze(node->_if.elseBlock, context);
		} break;

		case NodeType_While:
		{
			Analyze(node->_while.condition, context);
			Analyze(node->_while.body, context);
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
			Analyze(node->binary.lhs, context);
			Analyze(node->binary.rhs, context);
		} break;

		case NodeType_Print:
		{
			Analyze(node->print.expr, context);
		} break;

		case NodeType_Func:
		{
			Analyze(node->func.body, context);
		} break;

		case NodeType_Call:
		{
			Function *function = LookupFunction(funcTable, node->call.name);
			if (!function)
			{
				Error(context, node, STR_FMT_QUOTED ": identifier not found", STR_ARG(node->call.name));
			}
		} break;

		case NodeType_Return:
		{
			Analyze(node->ret.expr, context);
		} break;

		case NodeType_Number:
		{
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
			DeclareFunction(&funcTable, functionDef->func.name);
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

		Analyze(functionDef, context);

		int stackSize = (symTable.maxStackSize + 15) & ~15;
		functionBody->block.stackSize = stackSize;
	}
}
