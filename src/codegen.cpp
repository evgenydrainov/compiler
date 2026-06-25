#include "codegen.h"
#include <stdio.h>
#include <stdarg.h>

internal void
GenerateStatement(Node *node,
				  FILE *out,
				  CodegenContext *context);

internal void
GenerateBlock(Node *baseNode,
			  FILE *out,
			  CodegenContext *context)
{
	BlockNode *node = As<BlockNode>(baseNode);

	for (Node *it : node->statements)
	{
		GenerateStatement(it, out, context);
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
GenerateExpression(Node *baseNode,
				   FILE *out,
				   CodegenContext *context);

internal void
GenerateLValueAddress(Node *baseNode,
					  FILE *out,
					  CodegenContext *context)
{
	switch (baseNode->kind)
	{
		case NodeKind_Var:
		{
			VarNode *node = As<VarNode>(baseNode);

			fprintf(out, "    lea rax, [rbp - %d]\t; load address of variable " STR_FMT_QUOTED "\n",
					node->stackOffset,
					STR_ARG(node->name));
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeKind_Deref:
		{
			DerefNode *node = As<DerefNode>(baseNode);

			GenerateExpression(node->what, out, context);
		} break;

		case NodeKind_FieldAccess:
		{
			FieldAccessNode *node = As<FieldAccessNode>(baseNode);

			GenerateLValueAddress(node->expr, out, context);
			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    add rax, %d\n", node->fieldOffset);
			WritePush(out, context, "    push rax\n");
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

internal void
GenerateBinaryExpression(Node *baseNode,
						 FILE *out,
						 CodegenContext *context)
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
			GenerateExpression(node->lhs, out, context);
			GenerateExpression(node->rhs, out, context);

			WritePop(out, context, "    pop rcx\n");
			WritePop(out, context, "    pop rax\n");

			if (node->op == BinaryOp_Add)
			{
				fprintf(out, "    add rax, rcx\t; perform addition\n");
			}
			else if (node->op == BinaryOp_Subtract)
			{
				fprintf(out, "    sub rax, rcx\t; perform subtraction\n");
			}
			else if (node->op == BinaryOp_Multiply)
			{
				fprintf(out, "    imul rax, rcx\t; perform multiplication\n");
			}
			else if (node->op == BinaryOp_Divide)
			{
				fprintf(out, "    cqo     \t\t;\n");
				fprintf(out, "    idiv rcx\t\t; perform division\n");
			}
			else if (node->op == BinaryOp_Modulo)
			{
				fprintf(out, "    cqo\n");
				fprintf(out, "    idiv rcx\n");
				fprintf(out, "    mov rax, rdx\n");
			}
			else
			{
				Assert(false);
			}

			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case BinaryOp_Greater:
		case BinaryOp_Less:
		case BinaryOp_EqualEqual:
		case BinaryOp_GreaterEqual:
		case BinaryOp_LessEqual:
		case BinaryOp_NotEqual:
		{
			GenerateExpression(node->lhs, out, context);
			GenerateExpression(node->rhs, out, context);

			const char *setccInstruction = "";
			if (node->op == BinaryOp_Greater)
			{
				setccInstruction = "setg";
			}
			else if (node->op == BinaryOp_Less)
			{
				setccInstruction = "setl";
			}
			else if (node->op == BinaryOp_EqualEqual)
			{
				setccInstruction = "sete";
			}
			else if (node->op == BinaryOp_GreaterEqual)
			{
				setccInstruction = "setge";
			}
			else if (node->op == BinaryOp_LessEqual)
			{
				setccInstruction = "setle";
			}
			else if (node->op == BinaryOp_NotEqual)
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

		case BinaryOp_LogicalAnd:
		{
			int uniqueId = context->uniqueLabelId++;

			GenerateExpression(node->lhs, out, context);

			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    cmp rax, 0\n");
			fprintf(out, "    je .and_false_%d\n", uniqueId);
			fprintf(out, "\n");

			GenerateExpression(node->rhs, out, context);

			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    cmp rax, 0\n");
			fprintf(out, "    je .and_false_%d\n", uniqueId);
			fprintf(out, "\n");

			fprintf(out, "    mov rax, 1\n");
			fprintf(out, "    jmp .and_end_%d\n", uniqueId);
			fprintf(out, ".and_false_%d:\n", uniqueId);
			fprintf(out, "    mov rax, 0\n");
			fprintf(out, ".and_end_%d:\n", uniqueId);
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case BinaryOp_LogicalOr:
		{
			int uniqueId = context->uniqueLabelId++;

			GenerateExpression(node->lhs, out, context);

			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    cmp rax, 0\n");
			fprintf(out, "    jne .or_true_%d\n", uniqueId);
			fprintf(out, "\n");

			GenerateExpression(node->rhs, out, context);

			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    cmp rax, 0\n");
			fprintf(out, "    jne .or_true_%d\n", uniqueId);
			fprintf(out, "\n");

			fprintf(out, "    mov rax, 0\n");
			fprintf(out, "    jmp .or_end_%d\n", uniqueId);
			fprintf(out, ".or_true_%d:\n", uniqueId);
			fprintf(out, "    mov rax, 1\n");
			fprintf(out, ".or_end_%d:\n", uniqueId);
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;
	}
}

internal void
GenerateExpression(Node *baseNode,
				   FILE *out,
				   CodegenContext *context)
{
	switch (baseNode->kind)
	{
		default:
		{
			Assert(false);
		} break;

		case NodeKind_Number:
		{
			NumberNode *node = As<NumberNode>(baseNode);

			fprintf(out, "    mov rax, %lld\t\t; load integer literal\n", node->int64Value);
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeKind_Bool:
		{
			BoolNode *node = As<BoolNode>(baseNode);

			fprintf(out, "    mov rax, %d\t\t; load boolean literal\n", (int)node->boolValue);
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeKind_Binary:
		{
			GenerateBinaryExpression(baseNode, out, context);
		} break;

		case NodeKind_Var:
		case NodeKind_Deref:
		case NodeKind_FieldAccess:
		{
			GenerateLValueAddress(baseNode, out, context);
			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    mov rax, [rax]\n");
			WritePush(out, context, "    push rax\n");
			fprintf(out, "\n");
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(baseNode);

			const char *paramRegs[] =
			{
				"rcx",
				"rdx",
				"r8",
				"r9",
			};

			Assert(node->numExpressions <= 4);

			int numArgumentsInRegs = Min(node->numExpressions, 4);

			for (int i = 0;
				 i < numArgumentsInRegs;
				 i++)
			{
				GenerateExpression(node->expressions[i], out, context);
			}

			for (int i = numArgumentsInRegs;
				 i--;)
			{
				WritePop(out, context, "    pop %s\t\t\t; put argument\n", paramRegs[i]);
			}
			fprintf(out, "\n");

			PushShadowSpace(out, context);
			fprintf(out, "    call " STR_FMT "\n", STR_ARG(node->name));
			PopShadowSpace(out, context);
			WritePush(out, context, "    push rax\t\t; push the function return value\n");
			fprintf(out, "\n");
		} break;

		case NodeKind_AddressOf:
		{
			AddressOfNode *node = As<AddressOfNode>(baseNode);

			GenerateLValueAddress(node->what, out, context);
		} break;
	}
}

internal void
GenerateStatement(Node *baseNode,
				  FILE *out,
				  CodegenContext *context)
{
	switch (baseNode->kind)
	{
		case NodeKind_VarDecl:
		{
			VarDeclNode *node = As<VarDeclNode>(baseNode);

			if (node->expr)
			{
				GenerateExpression(node->expr, out, context);

				WritePop(out, context, "    pop rax\t\t\t; store into " STR_FMT_QUOTED "\n", STR_ARG(node->name));
				fprintf(out, "    mov [rbp - %d], rax\n", node->stackOffset);
				fprintf(out, "\n");
			}
			else
			{
				// variable is uninitialized
			}
		} break;

		case NodeKind_Assign:
		{
			AssignNode *node = As<AssignNode>(baseNode);

			GenerateExpression(node->rhs, out, context);
			GenerateLValueAddress(node->lhs, out, context);

			WritePop(out, context, "    pop rcx\n");
			WritePop(out, context, "    pop rax\n");
			fprintf(out, "    mov [rcx], rax\n");
			fprintf(out, "\n");
		} break;

		case NodeKind_If:
		{
			IfNode *node = As<IfNode>(baseNode);

			int uniqueId = context->uniqueLabelId++;

			if (node->elseBlock)
			{
				GenerateExpression(node->condition, out, context);

				WritePop(out, context, "    pop rax\t\t; load the comparison result\n");
				fprintf(out, "    cmp rax, 0\n");
				fprintf(out, "    je .else_%d\n", uniqueId);
				fprintf(out, "\n");

				GenerateStatement(node->thenBlock, out, context);

				fprintf(out, "    jmp .end_%d\n", uniqueId);
				fprintf(out, ".else_%d:\n", uniqueId);

				GenerateStatement(node->elseBlock, out, context);

				fprintf(out, ".end_%d:\n", uniqueId);
				fprintf(out, "\n");
			}
			else
			{
				GenerateExpression(node->condition, out, context);

				WritePop(out, context, "    pop rax\t\t; load the comparison result\n");
				fprintf(out, "    cmp rax, 0\n");
				fprintf(out, "    je .end_%d\n", uniqueId);
				fprintf(out, "\n");

				GenerateStatement(node->thenBlock, out, context);

				fprintf(out, ".end_%d:\n", uniqueId);
				fprintf(out, "\n");
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);

			int uniqueId = context->uniqueLabelId++;

			fprintf(out, ".loop_%d:\n", uniqueId);

			GenerateExpression(node->condition, out, context);

			WritePop(out, context, "    pop rax\t\t; load the comparison result\n");
			fprintf(out, "    cmp rax, 0\n");
			fprintf(out, "    je .end_%d\n", uniqueId);
			fprintf(out, "\n");

			GenerateBlock(node->body, out, context);

			fprintf(out, "    jmp .loop_%d\n", uniqueId);
			fprintf(out, ".end_%d:\n", uniqueId);
			fprintf(out, "\n");
		} break;

		case NodeKind_Print:
		{
			PrintNode *node = As<PrintNode>(baseNode);

			GenerateExpression(node->expr, out, context);

			WritePop(out, context, "    pop rax\t\t\t; store expression result into rax\n");
			fprintf(out, "\n");

			fprintf(out, "    lea rcx, [rel builtin_print_format]\t\t; put 1st argument into rcx\n");
			fprintf(out, "    mov rdx, rax\t\t\t; put 2nd argument into rdx\n");
			PushShadowSpace(out, context);
			fprintf(out, "    call printf\n");
			PopShadowSpace(out, context);
			fprintf(out, "\n");
		} break;

		case NodeKind_Block:
		{
			BlockNode *node = As<BlockNode>(baseNode);

			GenerateBlock(node, out, context);
		} break;

		case NodeKind_Return:
		{
			ReturnNode *node = As<ReturnNode>(baseNode);

			if (node->expr)
			{
				GenerateExpression(node->expr, out, context);

				WritePop(out, context, "    pop rax\t\t\t; store expression result into rax\n");
			}
			else
			{
				// bare return;
			}
			
			fprintf(out, "    jmp .epilogue\t\t; return\n");
			fprintf(out, "\n");
		} break;

		default:
		{
			GenerateExpression(baseNode, out, context);

			WritePop(out, context, "    pop rax\t\t\t; discard the result\n");
			fprintf(out, "\n");
		} break;
	}
}

internal void
GenerateTopLevelStatement(Node *baseNode,
						  FILE *out,
						  CodegenContext *context)
{
	switch (baseNode->kind)
	{
		case NodeKind_Func:
		{
			FuncNode *node = As<FuncNode>(baseNode);

			BlockNode *functionBody = As<BlockNode>(node->body);

			fprintf(out, STR_FMT ":\n", STR_ARG(node->name));
			fprintf(out, "    push rbp\n"); // does not affect context->stackDepth
			fprintf(out, "    mov rbp, rsp\n");
			fprintf(out, "    sub rsp, %d\n", functionBody->stackSize);
			fprintf(out, "\n");

			const char *paramRegs[] =
			{
				"rcx",
				"rdx",
				"r8",
				"r9",
			};

			Assert(node->numParams <= 4);

			int numParamsInRegs = Min(node->numParams, 4);

			for (int i = 0;
				 i < numParamsInRegs;
				 i++)
			{
				ParamNode *param = As<ParamNode>(node->params[i]);

				fprintf(out, "    mov [rbp - %d], %s\t\t; unpack argument\n", param->stackOffset, paramRegs[i]);
			}
			fprintf(out, "\n");

			GenerateBlock(node->body, out, context);

			fprintf(out, ".epilogue:\n");
			fprintf(out, "    mov rsp, rbp\n");
			fprintf(out, "    pop rbp\n"); // does not affect context->stackDepth
			fprintf(out, "    ret\n");
			fprintf(out, "\n");

			Assert(context->stackDepth == 0);
		} break;

		case NodeKind_StructDecl:
		{
			// do nothing
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

void
Generate_x86_64(Node *_program,
				FILE *out,
				CodegenContext *context)
{
	fprintf(out, "default rel\n");
	fprintf(out, "\n");

	BlockNode *program = As<BlockNode>(_program);

	for (Node *it : program->statements)
	{
		if (it->kind == NodeKind_Func)
		{
			FuncNode *node = As<FuncNode>(it);
			fprintf(out, "global " STR_FMT "\n", STR_ARG(node->name));
		}
	}
	fprintf(out, "\n");

	fprintf(out, "extern printf\n");
	fprintf(out, "\n");

	fprintf(out, "section .data\n");
	fprintf(out, "builtin_print_format: db \"%%d\", 10, 0\t\t; 10 is newline, 0 is null terminator\n");
	fprintf(out, "\n");

	fprintf(out, "section .text\n");

	for (Node *it : program->statements)
	{
		GenerateTopLevelStatement(it, out, context);
	}

	Assert(context->stackDepth == 0);
}
