#include "semantic_pass.h"
#include "symbol_table.h"
#include <stdio.h>

internal void
SemanticPassInner(AstNode *node,
				  SymbolTable *table,
				  SemanticPassContext *context)
{
	if (!node)
	{
		return;
	}

	switch (node->type)
	{
		case NodeType_Block:
		{
			int numSymbols = table->count;
			int stackSize = table->stackSize;
			int scopeStart = table->scopeStart;

			table->scopeStart = table->count;

			for (int i = 0;
				 i < node->block.numStatements;
				 i++)
			{
				SemanticPassInner(node->block.statements[i], table, context);
			}

			table->count = numSymbols;
			table->stackSize = stackSize;
			table->scopeStart = scopeStart;
		} break;

		case NodeType_VarDecl:
		{
			SemanticPassInner(node->assign.expr, table, context);

			if (LookupSymbol(table, node->assign.name, table->scopeStart))
			{
				fprintf(stderr, "'" STR_FMT "': redefinition\n", STR_ARG(node->assign.name));
				context->hadError = true;
			}
			else
			{
				Symbol *symbol = DeclareSymbol(table, node->assign.name);
				node->assign.stackOffset = symbol->offset;
			}
		} break;

		case NodeType_Assign:
		{
			SemanticPassInner(node->assign.expr, table, context);

			Symbol *symbol = LookupSymbol(table, node->assign.name, 0);
			if (symbol)
			{
				node->assign.stackOffset = symbol->offset;
			}
			else
			{
				fprintf(stderr, "'" STR_FMT "': undeclared identifier\n", STR_ARG(node->assign.name));
				context->hadError = true;
			}
		} break;

		case NodeType_Var:
		{
			Symbol *symbol = LookupSymbol(table, node->var.name, 0);
			if (symbol)
			{
				node->var.stackOffset = symbol->offset;
			}
			else
			{
				fprintf(stderr, "'" STR_FMT "': undeclared identifier\n", STR_ARG(node->var.name));
				context->hadError = true;
			}
		} break;

		case NodeType_If:
		{
			SemanticPassInner(node->_if.condition, table, context);
			SemanticPassInner(node->_if.thenBlock, table, context);
			SemanticPassInner(node->_if.elseBlock, table, context);
		} break;

		case NodeType_While:
		{
			SemanticPassInner(node->_while.condition, table, context);
			SemanticPassInner(node->_while.body, table, context);
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
			SemanticPassInner(node->binary.lhs, table, context);
			SemanticPassInner(node->binary.rhs, table, context);
		} break;

		case NodeType_Print:
		{
			SemanticPassInner(node->print.expr, table, context);
		} break;

		case NodeType_Number: {} break;
	}
}

void
SemanticPass(AstNode *program,
			 SemanticPassContext *context)
{
	SymbolTable table = {};
	SemanticPassInner(program, &table, context);

	int localSize = (table.maxStackSize + 15) & ~15;
	program->block.stackSize = localSize;
}
