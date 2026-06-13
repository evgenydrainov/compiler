#include "codegen.h"

internal void
GenerateStatement(AstNode *node,
				  FILE *out,
				  CodegenContext *context);

internal void
GenerateBlock(AstNode *node,
			  FILE *out,
			  CodegenContext *context)
{
	Assert(node->type == NodeType_Block);

	for (int i = 0;
		 i < node->block.numStatements;
		 i++)
	{
		GenerateStatement(node->block.statements[i], out, context);
	}
}

internal void
GenerateExpression(AstNode *node,
				   FILE *out,
				   CodegenContext *context)
{
	switch (node->type)
	{
		case NodeType_Number:
		{
			fprintf(out, "    mov rax, %d\t\t; load integer literal\n", node->number.value);
			fprintf(out, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Add:
		case NodeType_Subtract:
		case NodeType_Multiply:
		case NodeType_Divide:
		{
			GenerateExpression(node->binary.lhs, out, context);
			GenerateExpression(node->binary.rhs, out, context);

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
			fprintf(out, "    mov rax, [rbp - %d]\t; load variable " STR_FMT_QUOTED "\n",
					node->var.stackOffset,
					STR_ARG(node->var.name));
			fprintf(out, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Greater:
		case NodeType_Less:
		case NodeType_EqualEqual:
		case NodeType_GreaterEqual:
		case NodeType_LessEqual:
		case NodeType_NotEqual:
		{
			GenerateExpression(node->binary.lhs, out, context);
			GenerateExpression(node->binary.rhs, out, context);

			const char *setccInstruction = "";
			if (node->type == NodeType_Greater)
			{
				setccInstruction = "setg";
			}
			else if (node->type == NodeType_Less)
			{
				setccInstruction = "setl";
			}
			else if (node->type == NodeType_EqualEqual)
			{
				setccInstruction = "sete";
			}
			else if (node->type == NodeType_GreaterEqual)
			{
				setccInstruction = "setge";
			}
			else if (node->type == NodeType_LessEqual)
			{
				setccInstruction = "setle";
			}
			else if (node->type == NodeType_NotEqual)
			{
				setccInstruction = "setne";
			}
			else
			{
				Assert(false);
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
				  FILE *out,
				  CodegenContext *context)
{
	switch (node->type)
	{
		case NodeType_VarDecl:
		case NodeType_Assign:
		{
			GenerateExpression(node->assign.expr, out, context);

			fprintf(out, "    pop rax\t\t\t; store into " STR_FMT_QUOTED "\n", STR_ARG(node->assign.name));
			fprintf(out, "    mov [rbp - %d], rax\n", node->assign.stackOffset);
			fprintf(out, "\n");
		} break;

		case NodeType_If:
		{
			int uniqueId = context->uniqueLabelId++;

			if (node->_if.elseBlock)
			{
				GenerateExpression(node->_if.condition, out, context);

				fprintf(out, "    pop rax\t\t; load the comparison result\n");
				fprintf(out, "    cmp rax, 0\n");
				fprintf(out, "    je .else_%d\n", uniqueId);
				fprintf(out, "\n");

				GenerateBlock(node->_if.thenBlock, out, context);

				fprintf(out, "    jmp .end_%d\n", uniqueId);
				fprintf(out, ".else_%d:\n", uniqueId);

				GenerateBlock(node->_if.elseBlock, out, context);

				fprintf(out, ".end_%d:\n", uniqueId);
				fprintf(out, "\n");
			}
			else
			{
				GenerateExpression(node->_if.condition, out, context);

				fprintf(out, "    pop rax\t\t; load the comparison result\n");
				fprintf(out, "    cmp rax, 0\n");
				fprintf(out, "    je .end_%d\n", uniqueId);
				fprintf(out, "\n");

				GenerateBlock(node->_if.thenBlock, out, context);

				fprintf(out, ".end_%d:\n", uniqueId);
				fprintf(out, "\n");
			}
		} break;

		case NodeType_While:
		{
			int uniqueId = context->uniqueLabelId++;

			fprintf(out, ".loop_%d:\n", uniqueId);

			GenerateExpression(node->_while.condition, out, context);

			fprintf(out, "    pop rax\t\t; load the comparison result\n");
			fprintf(out, "    cmp rax, 0\n");
			fprintf(out, "    je .end_%d\n", uniqueId);
			fprintf(out, "\n");

			GenerateBlock(node->_while.body, out, context);

			fprintf(out, "    jmp .loop_%d\n", uniqueId);
			fprintf(out, ".end_%d:\n", uniqueId);
			fprintf(out, "\n");
		} break;

		case NodeType_Print:
		{
			GenerateExpression(node->print.expr, out, context);

			fprintf(out, "    pop rax\t\t\t; store expression result into rax\n");
			fprintf(out, "\n");

			fprintf(out, "    lea rcx, [rel format]\t\t; put 1st argument into rcx\n");
			fprintf(out, "    mov rdx, rax\t\t\t; put 2nd argument into rdx\n");
			fprintf(out, "    sub rsp, 32\t\t; push shadow space\n");
			fprintf(out, "    call printf\n");
			fprintf(out, "    add rsp, 32\t\t; pop shadow space\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Block:
		{
			GenerateBlock(node, out, context);
		} break;

		default:
		{
			GenerateExpression(node, out, context);

			fprintf(out, "    pop rax\t\t\t; discard the result\n");
			fprintf(out, "\n");
		} break;
	}
}

void
Generate_x86_64(AstNode *program,
				FILE *out,
				CodegenContext *context)
{
	int localSize = program->block.stackSize;

	fprintf(out, "default rel\n");
	fprintf(out, "global main\n");
	fprintf(out, "extern printf\n");
	fprintf(out, "\n");

	fprintf(out, "section .data\n");
	fprintf(out, "format: db \"%%d\", 10, 0\t\t; 10 is newline, 0 is null terminator\n");
	fprintf(out, "\n");

	fprintf(out, "section .text\n");
	fprintf(out, "main:\n");
	fprintf(out, "    push rbp\n");
	fprintf(out, "    mov rbp, rsp\n");
	fprintf(out, "    sub rsp, %d\n", localSize);
	fprintf(out, "\n");

	GenerateBlock(program, out, context);

	fprintf(out, "    mov rsp, rbp\n");
	fprintf(out, "    pop rbp\n");
	fprintf(out, "    ret\n");
}
