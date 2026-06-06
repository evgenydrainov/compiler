#include "codegen.h"

internal void
GenerateExpression(AstNode *node, FILE *out)
{
	switch (node->type)
	{
		case NodeType_Number:
		{
			fprintf(out, "    mov rax, %d\n", node->numberValue);
			fprintf(out, "    push rax\n");
		} break;

		case NodeType_Add:
		case NodeType_Subtract:
		case NodeType_Multiply:
		case NodeType_Divide:
		{
			GenerateExpression(node->lhs, out);
			GenerateExpression(node->rhs, out);
			fprintf(out, "    pop rcx\n");
			fprintf(out, "    pop rax\n");

			switch (node->type)
			{
				case NodeType_Add:
				{
					fprintf(out, "    add rax, rcx\n");
				} break;

				case NodeType_Subtract:
				{
					fprintf(out, "    sub rax, rcx\n");
				} break;

				case NodeType_Multiply:
				{
					fprintf(out, "    imul rax, rcx\n");
				} break;

				case NodeType_Divide:
				{
					fprintf(out, "    cqo\n");
					fprintf(out, "    idiv rcx\n");
				} break;

				default: {} break;
			}

			fprintf(out, "    push rax\n");
		} break;
	}
}

void
Generate_x86_64(AstNode *root, FILE *out)
{
	fprintf(out, "default rel\n");
	fprintf(out, "global main\n");
	fprintf(out, "section .text\n");
	fprintf(out, "main:\n");
	fprintf(out, "    sub rsp, 40\n");

	GenerateExpression(root, out);

	fprintf(out, "    pop rax\n");
	fprintf(out, "    add rsp, 40\n");
	fprintf(out, "    ret\n");
}
