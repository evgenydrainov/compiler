#include "codegen.h"

struct Symbol
{
	string name;
	int offset;
};

struct SymbolTable
{
	Symbol symbols[256];
	int count;
	int stackSize;
};

internal Symbol *
LookupSymbol(SymbolTable *table, string name)
{
	Symbol *result = nullptr;
	for (int i = 0;
		 i < table->count;
		 i++)
	{
		Symbol *symbol = &table->symbols[i];
		if (symbol->name == name)
		{
			result = symbol;
			break;
		}
	}

	return result;
}

internal Symbol *
DeclareSymbol(SymbolTable *table, string name)
{
	table->stackSize += 8;

	Assert(table->count < ArrayCount(table->symbols));

	Symbol *symbol = &table->symbols[table->count++];
	*symbol = {};
	symbol->name = name;
	symbol->offset = table->stackSize;

	return symbol;
}

internal void
GenerateStatement(AstNode *node,
				  SymbolTable *table,
				  FILE *out,
				  CodegenContext *context);

internal void
GenerateBlock(AstNode *node,
			  SymbolTable *table,
			  FILE *out,
			  CodegenContext *context)
{
	Assert(node->type == NodeType_Block);

	for (int i = 0;
		 i < node->numStatements;
		 i++)
	{
		GenerateStatement(node->statements[i], table, out, context);
	}
}

internal void
GenerateExpression(AstNode *node,
				   SymbolTable *table,
				   FILE *out,
				   CodegenContext *context)
{
	switch (node->type)
	{
		case NodeType_Number:
		{
			fprintf(out, "    mov rax, %d\t\t; load integer literal\n", node->numberValue);
			fprintf(out, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Add:
		case NodeType_Subtract:
		case NodeType_Multiply:
		case NodeType_Divide:
		{
			GenerateExpression(node->lhs, table, out, context);
			GenerateExpression(node->rhs, table, out, context);

			fprintf(out, "    pop rcx\n");
			fprintf(out, "    pop rax\n");

			switch (node->type)
			{
				case NodeType_Add:
				{
					fprintf(out, "    add rax, rcx\t; perform addition\n");
				} break;

				case NodeType_Subtract:
				{
					fprintf(out, "    sub rax, rcx\t; perform subtraction\n");
				} break;

				case NodeType_Multiply:
				{
					fprintf(out, "    imul rax, rcx\t; perform multiplication\n");
				} break;

				case NodeType_Divide:
				{
					fprintf(out, "    cqo     \t\t; perform division\n");
					fprintf(out, "    idiv rcx\t\t;\n");
				} break;

				default:
				{
					Assert(!"node->type not implemented");
				} break;
			}

			fprintf(out, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Var:
		{
			Symbol *symbol = LookupSymbol(table, node->name);
			Assert(symbol);

			fprintf(out, "    mov rax, [rbp - %d]\t; load variable '" STR_FMT "'\n",
					symbol->offset,
					STR_ARG(symbol->name));
			fprintf(out, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeType_If:
		{
			int uniqueId = context->uniqueLabelId++;

			if (node->elseBlock)
			{
				GenerateExpression(node->condition, table, out, context);

				fprintf(out, "    pop rax\t\t; load the comparison result\n");
				fprintf(out, "    cmp rax, 0\n");
				fprintf(out, "    je .else_%d\n", uniqueId);
				fprintf(out, "\n");

				GenerateBlock(node->thenBlock, table, out, context);

				fprintf(out, "    jmp .end_%d\n", uniqueId);
				fprintf(out, ".else_%d:\n", uniqueId);

				GenerateBlock(node->elseBlock, table, out, context);

				fprintf(out, ".end_%d:\n", uniqueId);
			}
			else
			{
				GenerateExpression(node->condition, table, out, context);

				fprintf(out, "    pop rax\t\t; load the comparison result\n");
				fprintf(out, "    cmp rax, 0\n");
				fprintf(out, "    je .end_%d\n", uniqueId);
				fprintf(out, "\n");

				GenerateBlock(node->thenBlock, table, out, context);

				fprintf(out, ".end_%d:\n", uniqueId);
			}

			fprintf(out, "    push 123\t\t; push dummy value\n");
			fprintf(out, "\n");
		} break;

		case NodeType_While:
		{
			int uniqueId = context->uniqueLabelId++;

			fprintf(out, ".loop_%d:\n", uniqueId);

			GenerateExpression(node->condition, table, out, context);

			fprintf(out, "    cmp rax, 0\n");
			fprintf(out, "    je .end_%d\n", uniqueId);
			fprintf(out, "\n");

			GenerateBlock(node->thenBlock, table, out, context);

			fprintf(out, "    jmp .loop_%d\n", uniqueId);
			fprintf(out, ".end_%d:\n", uniqueId);

			fprintf(out, "    push 123\t\t; push dummy value\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Greater:
		case NodeType_Less:
		case NodeType_Equal:
		{
			GenerateExpression(node->lhs, table, out, context);
			GenerateExpression(node->rhs, table, out, context);

			const char *setccInstruction = "";
			if (node->type == NodeType_Greater)
			{
				setccInstruction = "setg";
			}
			else if (node->type == NodeType_Less)
			{
				setccInstruction = "setl";
			}
			else if (node->type == NodeType_Equal)
			{
				setccInstruction = "sete";
			}

			fprintf(out, "    pop rcx\n");
			fprintf(out, "    pop rax\n");
			fprintf(out, "    cmp rax, rcx\n");
			fprintf(out, "    %s al\n", setccInstruction);
			fprintf(out, "    movzx rax, al\n");
			fprintf(out, "    push rax\t\t; push the comparison result\n");
			fprintf(out, "\n");
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

internal void
GenerateStatement(AstNode *node,
				  SymbolTable *table,
				  FILE *out,
				  CodegenContext *context)
{
	switch (node->type)
	{
		case NodeType_VarDecl:
		case NodeType_Assign:
		{
			Symbol *symbol = LookupSymbol(table, node->name);
			Assert(symbol);

			GenerateExpression(node->rhs, table, out, context);

			fprintf(out, "    pop rax\t\t\t; store into '" STR_FMT "'\n", STR_ARG(symbol->name));
			fprintf(out, "    mov [rbp - %d], rax\n", symbol->offset);
			fprintf(out, "\n");
		} break;

		default:
		{
			GenerateExpression(node, table, out, context);

			fprintf(out, "    pop rax\t\t\t; discard the result\n");
			fprintf(out, "\n");
		} break;
	}
}

internal void
CollectSymbols(AstNode *node, SymbolTable *table)
{
	if (node->type == NodeType_Block)
	{
		for (int i = 0;
			 i < node->numStatements;
			 i++)
		{
			CollectSymbols(node->statements[i], table);
		}
	}
	else if (node->type == NodeType_VarDecl)
	{
		if (!LookupSymbol(table, node->name))
		{
			DeclareSymbol(table, node->name);
		}
	}
}

void
Generate_x86_64(AstNode *program,
				FILE *out,
				CodegenContext *context)
{
	SymbolTable table = {};
	CollectSymbols(program, &table);

	int localSize = (table.stackSize + 15) & ~15;

	fprintf(out, "default rel\n");
	fprintf(out, "global main\n");
	fprintf(out, "section .text\n");
	fprintf(out, "main:\n");
	fprintf(out, "    push rbp\n");
	fprintf(out, "    mov rbp, rsp\n");
	fprintf(out, "    sub rsp, %d\n", localSize);
	fprintf(out, "\n");

	GenerateBlock(program, &table, out, context);

	fprintf(out, "    mov rsp, rbp\n");
	fprintf(out, "    pop rbp\n");
	fprintf(out, "    ret\n");
}
