#include "codegen.h"
#include <stdio.h>
#include <stdarg.h>

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
WritePush(FILE *out,
		  CodegenContext *context,
		  const char *format,
		  ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(out, format, args);

	va_end(args);

	context->stackDepth++;
}

internal void
WritePop(FILE *out,
		 CodegenContext *context,
		 const char *format,
		 ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(out, format, args);

	va_end(args);

	context->stackDepth--;
}

internal void
PushShadowSpace(FILE *out,
				CodegenContext *context)
{
	int numBytes = 32;
	if (context->stackDepth % 2 == 1)
	{
		numBytes += 8;
	}

	fprintf(out, "    sub rsp, %d\t\t; push shadow space\n", numBytes);
}

internal void
PopShadowSpace(FILE *out,
			   CodegenContext *context)
{
	int numBytes = 32;
	if (context->stackDepth % 2 == 1)
	{
		numBytes += 8;
	}

	fprintf(out, "    add rsp, %d\t\t; pop shadow space\n", numBytes);
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
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Add:
		case NodeType_Subtract:
		case NodeType_Multiply:
		case NodeType_Divide:
		{
			GenerateExpression(node->binary.lhs, out, context);
			GenerateExpression(node->binary.rhs, out, context);

			WritePop(out, context, "    pop rcx\n");
			WritePop(out, context, "    pop rax\n");

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
					fprintf(out, "    cqo     \t\t;\n");
					fprintf(out, "    idiv rcx\t\t; perform division\n");
				} break;

				default:
				{
					Assert(!"node->type not implemented");
				} break;
			}

			WritePush(out, context, "    push rax\n");
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

			WritePop(out, context, "    pop rcx\n");
			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    cmp rax, rcx\n");
			fprintf(out, "    %s al\n", setccInstruction);
			fprintf(out, "    movzx rax, al\n");
			WritePush(out, context, "    push rax\t\t; push the comparison result\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Var:
		{
			fprintf(out, "    mov rax, [rbp - %d]\t; load variable " STR_FMT_QUOTED "\n",
					node->var.stackOffset,
					STR_ARG(node->var.name));
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeType_Call:
		{
			const char *paramRegs[] =
			{
				"rcx",
				"rdx",
				"r8",
				"r9",
			};

			Assert(node->call.numExpressions <= 4);

			int numArgumentsInRegs = Min(node->call.numExpressions, 4);

			for (int i = 0;
				 i < numArgumentsInRegs;
				 i++)
			{
				GenerateExpression(node->call.expressions[i], out, context);
			}

			for (int i = numArgumentsInRegs;
				 i--;)
			{
				WritePop(out, context, "    pop %s\t\t\t; put argument\n", paramRegs[i]);
			}
			fprintf(out, "\n");

			PushShadowSpace(out, context);
			fprintf(out, "    call " STR_FMT "\n", STR_ARG(node->call.name));
			PopShadowSpace(out, context);
			WritePush(out, context, "    push rax\t\t; push the function return value\n");
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
		{
			if (node->varDecl.expr)
			{
				GenerateExpression(node->varDecl.expr, out, context);

				WritePop(out, context, "    pop rax\t\t\t; store into " STR_FMT_QUOTED "\n", STR_ARG(node->varDecl.name));
				fprintf(out, "    mov [rbp - %d], rax\n", node->varDecl.stackOffset);
				fprintf(out, "\n");
			}
		} break;

		case NodeType_Assign:
		{
			GenerateExpression(node->assign.expr, out, context);

			WritePop(out, context, "    pop rax\t\t\t; store into " STR_FMT_QUOTED "\n", STR_ARG(node->assign.name));
			fprintf(out, "    mov [rbp - %d], rax\n", node->assign.stackOffset);
			fprintf(out, "\n");
		} break;

		case NodeType_If:
		{
			int uniqueId = context->uniqueLabelId++;

			if (node->_if.elseBlock)
			{
				GenerateExpression(node->_if.condition, out, context);

				WritePop(out, context, "    pop rax\t\t; load the comparison result\n");
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

				WritePop(out, context, "    pop rax\t\t; load the comparison result\n");
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

			WritePop(out, context, "    pop rax\t\t; load the comparison result\n");
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

			WritePop(out, context, "    pop rax\t\t\t; store expression result into rax\n");
			fprintf(out, "\n");

			fprintf(out, "    lea rcx, [rel builtin_print_format]\t\t; put 1st argument into rcx\n");
			fprintf(out, "    mov rdx, rax\t\t\t; put 2nd argument into rdx\n");
			PushShadowSpace(out, context);
			fprintf(out, "    call printf\n");
			PopShadowSpace(out, context);
			fprintf(out, "\n");
		} break;

		case NodeType_Block:
		{
			GenerateBlock(node, out, context);
		} break;

		case NodeType_Return:
		{
			if (node->ret.expr)
			{
				GenerateExpression(node->ret.expr, out, context);

				WritePop(out, context, "    pop rax\t\t\t; store expression result into rax\n");
			}
			
			fprintf(out, "    jmp .epilogue\t\t; return\n");
			fprintf(out, "\n");
		} break;

		default:
		{
			GenerateExpression(node, out, context);

			WritePop(out, context, "    pop rax\t\t\t; discard the result\n");
			fprintf(out, "\n");
		} break;
	}
}

internal void
GenerateTopLevelStatement(AstNode *node,
						  FILE *out,
						  CodegenContext *context)
{
	switch (node->type)
	{
		case NodeType_Func:
		{
			AstNode *functionBody = node->func.body;

			fprintf(out, STR_FMT ":\n", STR_ARG(node->func.name));
			fprintf(out, "    push rbp\n"); // does not affect context->stackDepth
			fprintf(out, "    mov rbp, rsp\n");
			fprintf(out, "    sub rsp, %d\n", functionBody->block.stackSize);
			fprintf(out, "\n");

			const char *paramRegs[] =
			{
				"rcx",
				"rdx",
				"r8",
				"r9",
			};

			Assert(node->func.numParams <= 4);

			int numParamsInRegs = Min(node->func.numParams, 4);

			for (int i = 0;
				 i < numParamsInRegs;
				 i++)
			{
				AstNode *param = node->func.params[i];

				fprintf(out, "    mov [rbp - %d], %s\t\t; unpack argument\n", param->param.stackOffset, paramRegs[i]);
			}
			fprintf(out, "\n");

			GenerateBlock(node->func.body, out, context);

			fprintf(out, ".epilogue:\n");
			fprintf(out, "    mov rsp, rbp\n");
			fprintf(out, "    pop rbp\n"); // does not affect context->stackDepth
			fprintf(out, "    ret\n");
			fprintf(out, "\n");

			Assert(context->stackDepth == 0);
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

void
Generate_x86_64(AstNode *program,
				FILE *out,
				CodegenContext *context)
{
	fprintf(out, "default rel\n");
	fprintf(out, "\n");

	Assert(program->type == NodeType_Block);
	for (int i = 0;
		 i < program->block.numStatements;
		 i++)
	{
		Assert(program->block.statements[i]->type == NodeType_Func);
		fprintf(out, "global " STR_FMT "\n", STR_ARG(program->block.statements[i]->func.name));
	}
	fprintf(out, "\n");

	fprintf(out, "extern printf\n");
	fprintf(out, "\n");

	fprintf(out, "section .data\n");
	fprintf(out, "builtin_print_format: db \"%%d\", 10, 0\t\t; 10 is newline, 0 is null terminator\n");
	fprintf(out, "\n");

	fprintf(out, "section .text\n");

	Assert(program->type == NodeType_Block);
	for (int i = 0;
		 i < program->block.numStatements;
		 i++)
	{
		GenerateTopLevelStatement(program->block.statements[i], out, context);
	}

	Assert(context->stackDepth == 0);
}
