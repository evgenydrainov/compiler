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
Emit(FILE *out,
	 CodegenContext *context,
	 const char *format,
	 ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(out, format, args);
	fprintf(out, "\n");

	va_end(args);

	string formatStr = { (char *)format, strlen(format) };
	TrimLeft(&formatStr);

	if (StartsWith(formatStr, "push "))
	{
		context->stackDepth++;
	}

	if (StartsWith(formatStr, "pop "))
	{
		context->stackDepth--;
	}
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

	Emit(out, context, "    sub rsp, %d\t\t; push shadow space", numBytes);
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

	Emit(out, context, "    add rsp, %d\t\t; pop shadow space", numBytes);
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

			Emit(out, context, "    lea rax, [rbp - %d]\t; load address of variable " STR_FMT_QUOTED,
				 node->stackOffset,
				 STR_ARG(node->name));
			Emit(out, context, "    push rax");
			Emit(out, context, "");
		} break;

		case NodeKind_Deref:
		{
			DerefNode *node = As<DerefNode>(baseNode);

			GenerateExpression(node->what, out, context);
		} break;

		case NodeKind_FieldAccess:
		{
			FieldAccessNode *node = As<FieldAccessNode>(baseNode);

			if (node->expr->inferredType.kind == TypeKind_Struct)
			{
				GenerateLValueAddress(node->expr, out, context);
			}
			else if (node->expr->inferredType.kind == TypeKind_Pointer)
			{
				GenerateExpression(node->expr, out, context);
			}
			else
			{
				Assert(false);
			}

			Emit(out, context, "    pop rax");
			Emit(out, context, "    add rax, %d\t\t; add offset of field " STR_FMT_QUOTED,
				 node->fieldOffset,
				 STR_ARG(node->fieldName));
			Emit(out, context, "    push rax");
		} break;

		case NodeKind_String:
		{
			StringNode *node = As<StringNode>(baseNode);

			Emit(out, context, "    lea rax, [rel string_literal_%d]", node->uniqueId);
			Emit(out, context, "    push rax");
			Emit(out, context, "");
		} break;

		case NodeKind_ArrayIndexAccess:
		{
			ArrayIndexAccessNode *node = As<ArrayIndexAccessNode>(baseNode);

			int size = 0;

			if (node->arrayExpr->inferredType.kind == TypeKind_Pointer)
			{
				GenerateExpression(node->arrayExpr, out, context);

				size = SizeOfType(*node->arrayExpr->inferredType.pointerTo);
			}
			else if (node->arrayExpr->inferredType.kind == TypeKind_Array)
			{
				GenerateLValueAddress(node->arrayExpr, out, context);

				size = SizeOfType(*node->arrayExpr->inferredType.arrayElementType);
			}
			else
			{
				Assert(false);
			}

			GenerateExpression(node->indexExpr, out, context);

			Emit(out, context, "    pop rcx");
			Emit(out, context, "    pop rbx");

			if (size == 1
				|| size == 2
				|| size == 4
				|| size == 8)
			{
				Emit(out, context, "    lea rax, [rbx + rcx * %d]", size);
				Emit(out, context, "    push rax");
			}
			else
			{
				Emit(out, context, "    imul rcx, rcx, %d", size);
				Emit(out, context, "    add rbx, rcx");
				Emit(out, context, "    push rbx");
			}
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
		case BinaryOp_ShiftLeft:
		case BinaryOp_ShiftRight:
		case BinaryOp_BitAnd:
		case BinaryOp_BitOr:
		case BinaryOp_BitXor:
		{
			GenerateExpression(node->lhs, out, context);
			GenerateExpression(node->rhs, out, context);

			Emit(out, context, "    pop rcx");
			Emit(out, context, "    pop rax");

			if (node->op == BinaryOp_Add)
			{
				Emit(out, context, "    add rax, rcx\t; perform addition");
			}
			else if (node->op == BinaryOp_Subtract)
			{
				Emit(out, context, "    sub rax, rcx\t; perform subtraction");
			}
			else if (node->op == BinaryOp_Multiply)
			{
				Emit(out, context, "    imul rax, rcx\t; perform multiplication");
			}
			else if (node->op == BinaryOp_Divide)
			{
				Emit(out, context, "    cqo     \t\t;");

				if (IsSignedInteger(node->inferredType))
				{
					Emit(out, context, "    idiv rcx\t\t; perform division");
				}
				else if (IsUnsignedInteger(node->inferredType))
				{
					Emit(out, context, "    div rcx\t\t; perform division");
				}
				else
				{
					Assert(false);
				}
			}
			else if (node->op == BinaryOp_Modulo)
			{
				Emit(out, context, "    cqo");

				if (IsSignedInteger(node->inferredType))
				{
					Emit(out, context, "    idiv rcx\t\t; perform division");
				}
				else if (IsUnsignedInteger(node->inferredType))
				{
					Emit(out, context, "    div rcx\t\t; perform division");
				}
				else
				{
					Assert(false);
				}

				Emit(out, context, "    mov rax, rdx");
			}
			else if (node->op == BinaryOp_ShiftLeft)
			{
				Emit(out, context, "    shl rax, cl");
			}
			else if (node->op == BinaryOp_ShiftRight)
			{
				if (IsSignedInteger(node->inferredType))
				{
					Emit(out, context, "    sar rax, cl");
				}
				else if (IsUnsignedInteger(node->inferredType))
				{
					Emit(out, context, "    shr rax, cl");
				}
				else
				{
					Assert(false);
				}
			}
			else if (node->op == BinaryOp_BitAnd)
			{
				Emit(out, context, "    and rax, rcx");
			}
			else if (node->op == BinaryOp_BitOr)
			{
				Emit(out, context, "    or rax, rcx");
			}
			else if (node->op == BinaryOp_BitXor)
			{
				Emit(out, context, "    xor rax, rcx");
			}
			else
			{
				Assert(false);
			}

			Emit(out, context, "    push rax");
			Emit(out, context, "");
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

			if (node->op == BinaryOp_EqualEqual)
			{
				setccInstruction = "sete";
			}
			else if (node->op == BinaryOp_NotEqual)
			{
				setccInstruction = "setne";
			}
			else
			{
				if (IsSignedInteger(node->lhs->inferredType))
				{
					if (node->op == BinaryOp_Greater)
					{
						setccInstruction = "setg";
					}
					else if (node->op == BinaryOp_Less)
					{
						setccInstruction = "setl";
					}
					else if (node->op == BinaryOp_GreaterEqual)
					{
						setccInstruction = "setge";
					}
					else if (node->op == BinaryOp_LessEqual)
					{
						setccInstruction = "setle";
					}
					else
					{
						Assert(false);
					}
				}
				else if (IsUnsignedInteger(node->lhs->inferredType))
				{
					if (node->op == BinaryOp_Greater)
					{
						setccInstruction = "seta";
					}
					else if (node->op == BinaryOp_Less)
					{
						setccInstruction = "setb";
					}
					else if (node->op == BinaryOp_GreaterEqual)
					{
						setccInstruction = "setae";
					}
					else if (node->op == BinaryOp_LessEqual)
					{
						setccInstruction = "setbe";
					}
					else
					{
						Assert(false);
					}
				}
				else
				{
					Assert(false);
				}
			}

			Emit(out, context, "    pop rcx");
			Emit(out, context, "    pop rax");
			Emit(out, context, "    cmp rax, rcx");
			Emit(out, context, "    %s al", setccInstruction);
			Emit(out, context, "    movzx rax, al");
			Emit(out, context, "    push rax\t\t; push the comparison result");
			Emit(out, context, "");
		} break;

		case BinaryOp_LogicalAnd:
		{
			int uniqueId = context->uniqueLabelId++;

			GenerateExpression(node->lhs, out, context);

			Emit(out, context, "    pop rax");
			Emit(out, context, "    cmp rax, 0");
			Emit(out, context, "    je .and_false_%d", uniqueId);
			Emit(out, context, "");

			GenerateExpression(node->rhs, out, context);

			Emit(out, context, "    pop rax");
			Emit(out, context, "    cmp rax, 0");
			Emit(out, context, "    je .and_false_%d", uniqueId);
			Emit(out, context, "");

			Emit(out, context, "    mov rax, 1");
			Emit(out, context, "    jmp .and_end_%d", uniqueId);
			Emit(out, context, ".and_false_%d:", uniqueId);
			Emit(out, context, "    mov rax, 0");
			Emit(out, context, ".and_end_%d:", uniqueId);
			Emit(out, context, "    push rax");
			Emit(out, context, "");
		} break;

		case BinaryOp_LogicalOr:
		{
			int uniqueId = context->uniqueLabelId++;

			GenerateExpression(node->lhs, out, context);

			Emit(out, context, "    pop rax");
			Emit(out, context, "    cmp rax, 0");
			Emit(out, context, "    jne .or_true_%d", uniqueId);
			Emit(out, context, "");

			GenerateExpression(node->rhs, out, context);

			Emit(out, context, "    pop rax");
			Emit(out, context, "    cmp rax, 0");
			Emit(out, context, "    jne .or_true_%d", uniqueId);
			Emit(out, context, "");

			Emit(out, context, "    mov rax, 0");
			Emit(out, context, "    jmp .or_end_%d", uniqueId);
			Emit(out, context, ".or_true_%d:", uniqueId);
			Emit(out, context, "    mov rax, 1");
			Emit(out, context, ".or_end_%d:", uniqueId);
			Emit(out, context, "    push rax");
			Emit(out, context, "");
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

			Emit(out, context, "    mov rax, %lld\t\t; load integer literal", node->int64Value);
			Emit(out, context, "    push rax");
			Emit(out, context, "");
		} break;

		case NodeKind_CString:
		{
			CStringNode *node = As<CStringNode>(baseNode);

			Emit(out, context, "    lea rax, [rel cstring_literal_%d]\t; load string literal", node->uniqueId);
			Emit(out, context, "    push rax");
			Emit(out, context, "");
		} break;

		case NodeKind_Bool:
		{
			BoolNode *node = As<BoolNode>(baseNode);

			Emit(out, context, "    mov rax, %d\t\t; load boolean literal", (int)node->boolValue);
			Emit(out, context, "    push rax");
			Emit(out, context, "");
		} break;

		case NodeKind_Binary:
		{
			GenerateBinaryExpression(baseNode, out, context);
		} break;

		case NodeKind_Unary:
		{
			UnaryNode *node = As<UnaryNode>(baseNode);

			GenerateExpression(node->expr, out, context);

			if (node->op == UnaryOp_Negate)
			{
				Emit(out, context, "    pop rax");
				Emit(out, context, "    neg rax");
				Emit(out, context, "    push rax");
			}
			else if (node->op == UnaryOp_LogicalNot)
			{
				Emit(out, context, "    pop rax");
				Emit(out, context, "    cmp rax, 0");
				Emit(out, context, "    sete al");
				Emit(out, context, "    movzx rax, al");
				Emit(out, context, "    push rax");
			}
			else if (node->op == UnaryOp_BitNegate)
			{
				Emit(out, context, "    pop rax");
				Emit(out, context, "    not rax");
				Emit(out, context, "    push rax");
			}
			else
			{
				Assert(false);
			}

			Emit(out, context, "");
		} break;

		case NodeKind_Var:
		case NodeKind_Deref:
		case NodeKind_FieldAccess:
		case NodeKind_ArrayIndexAccess:
		{
			int size = SizeOfType(baseNode->inferredType);

			GenerateLValueAddress(baseNode, out, context);
			Emit(out, context, "    pop rax");

			if (IsSignedInteger(baseNode->inferredType))
			{
				if (size == 8)
				{
					Emit(out, context, "    mov rax, qword [rax]");
				}
				else if (size == 4)
				{
					Emit(out, context, "    movsxd rax, dword [rax]");
				}
				else if (size == 2)
				{
					Emit(out, context, "    movsx rax, word [rax]");
				}
				else if (size == 1)
				{
					Emit(out, context, "    movsx rax, byte [rax]");
				}
				else
				{
					Assert(false);
				}
			}
			else
			{
				if (size == 8)
				{
					Emit(out, context, "    mov rax, qword [rax]");
				}
				else if (size == 4)
				{
					Emit(out, context, "    mov eax, dword [rax]");
				}
				else if (size == 2)
				{
					Emit(out, context, "    movzx rax, word [rax]");
				}
				else if (size == 1)
				{
					Emit(out, context, "    movzx rax, byte [rax]");
				}
				else
				{
					Assert(false);
				}
			}

			Emit(out, context, "    push rax");
			Emit(out, context, "");
		} break;

		case NodeKind_Cast:
		{
			CastNode *node = As<CastNode>(baseNode);

			GenerateExpression(node->what, out, context);
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

			int numArgumentsInRegs = Min(node->numExpressions, 4);
			int numArgumentsOnStack = node->numExpressions - numArgumentsInRegs;

			int padding = ((context->stackDepth + numArgumentsOnStack) % 2) ? 8 : 0;
			if (padding)
			{
				Emit(out, context, "    sub rsp, %d\t\t; align stack", padding);
			}

			for (int i = node->numExpressions;
				 i--;)
			{
				GenerateExpression(node->expressions[i], out, context);
			}

			for (int i = 0;
				 i < numArgumentsInRegs;
				 i++)
			{
				Emit(out, context, "    pop %s\t\t\t; put argument", paramRegs[i]);
			}
			Emit(out, context, "");

			Emit(out, context, "    sub rsp, 32\t\t; reserve shadow space");
			Emit(out, context, "    call " STR_FMT, STR_ARG(node->linkName));
			Emit(out, context, "    add rsp, 32\t\t; free shadow space");

			if (numArgumentsOnStack + padding/8 > 0)
			{
				int cleanup = numArgumentsOnStack*8 + padding;
				Emit(out, context, "    add rsp, %d\t\t; free stack arguments", cleanup);
				context->stackDepth -= numArgumentsOnStack;
			}

			Emit(out, context, "    push rax\t\t; push the function return value");
			Emit(out, context, "");
		} break;

		case NodeKind_AddressOf:
		{
			AddressOfNode *node = As<AddressOfNode>(baseNode);

			GenerateLValueAddress(node->what, out, context);
		} break;
	}
}

internal const char *
GetRegisterForTypeSize(Type type)
{
	int size = SizeOfType(type);
	if (size == 8)
	{
		return "rax";
	}
	else if (size == 4)
	{
		return "eax";
	}
	else if (size == 2)
	{
		return "ax";
	}
	else if (size == 1)
	{
		return "al";
	}

	Assert(false);
	return "rax";
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

			VarNode varNode = {};
			varNode.kind = NodeKind_Var;
			varNode.inferredType = node->type;
			varNode.name = node->name;
			varNode.stackOffset = node->stackOffset;

			if (node->expr)
			{
				AssignNode assignNode = {};
				assignNode.kind = NodeKind_Assign;
				assignNode.lhs = &varNode;
				assignNode.rhs = node->expr;

				GenerateStatement(&assignNode, out, context);
			}
			else
			{
				// initialize to zero by default

				int size = SizeOfType(varNode.inferredType);
				if (size > 8)
				{
					GenerateLValueAddress(&varNode, out, context);

					Emit(out, context, "    pop rdi");

					Assert(size % 8 == 0);

					for (int i = 0; i < size; i += 8)
					{
						Emit(out, context, "    mov qword [rdi + %d], 0", i);
					}

					Emit(out, context, "");
				}
				else
				{
					GenerateLValueAddress(&varNode, out, context);

					Emit(out, context, "    pop rcx");
					Emit(out, context, "    mov rax, 0");

					const char *reg = GetRegisterForTypeSize(varNode.inferredType);

					Emit(out, context, "    mov [rcx], %s", reg);
					Emit(out, context, "");
				}
			}
		} break;

		case NodeKind_Assign:
		{
			AssignNode *node = As<AssignNode>(baseNode);

			int size = SizeOfType(node->lhs->inferredType);
			if (size > 8)
			{
				Assert(SizeOfType(node->lhs->inferredType) == SizeOfType(node->rhs->inferredType));

				GenerateLValueAddress(node->rhs, out, context);
				GenerateLValueAddress(node->lhs, out, context);

				Emit(out, context, "    pop rdi");
				Emit(out, context, "    pop rsi");

				Assert(size % 8 == 0);

				for (int i = 0; i < size; i += 8)
				{
					Emit(out, context, "    mov rax, [rsi + %d]", i);
					Emit(out, context, "    mov [rdi + %d], rax", i);
				}

				Emit(out, context, "");
			}
			else
			{
				GenerateExpression(node->rhs, out, context);
				GenerateLValueAddress(node->lhs, out, context);

				Emit(out, context, "    pop rcx");
				Emit(out, context, "    pop rax");

				const char *reg = GetRegisterForTypeSize(node->lhs->inferredType);

				Emit(out, context, "    mov [rcx], %s", reg);
				Emit(out, context, "");
			}
		} break;

		case NodeKind_If:
		{
			IfNode *node = As<IfNode>(baseNode);

			int uniqueId = context->uniqueLabelId++;

			if (node->elseBlock)
			{
				GenerateExpression(node->condition, out, context);

				Emit(out, context, "    pop rax\t\t; load the comparison result");
				Emit(out, context, "    cmp rax, 0");
				Emit(out, context, "    je .else_%d", uniqueId);
				Emit(out, context, "");

				GenerateStatement(node->thenBlock, out, context);

				Emit(out, context, "    jmp .end_%d", uniqueId);
				Emit(out, context, ".else_%d:", uniqueId);

				GenerateStatement(node->elseBlock, out, context);

				Emit(out, context, ".end_%d:", uniqueId);
				Emit(out, context, "");
			}
			else
			{
				GenerateExpression(node->condition, out, context);

				Emit(out, context, "    pop rax\t\t; load the comparison result");
				Emit(out, context, "    cmp rax, 0");
				Emit(out, context, "    je .end_%d", uniqueId);
				Emit(out, context, "");

				GenerateStatement(node->thenBlock, out, context);

				Emit(out, context, ".end_%d:", uniqueId);
				Emit(out, context, "");
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);

			int uniqueId = context->uniqueLabelId++;

			Emit(out, context, ".loop_%d:", uniqueId);

			GenerateExpression(node->condition, out, context);

			Emit(out, context, "    pop rax\t\t; load the comparison result");
			Emit(out, context, "    cmp rax, 0");
			Emit(out, context, "    je .end_%d", uniqueId);
			Emit(out, context, "");

			GenerateBlock(node->body, out, context);

			Emit(out, context, "    jmp .loop_%d", uniqueId);
			Emit(out, context, ".end_%d:", uniqueId);
			Emit(out, context, "");
		} break;

		case NodeKind_Print:
		{
			PrintNode *node = As<PrintNode>(baseNode);

			GenerateExpression(node->expr, out, context);

			Emit(out, context, "    pop rax\t\t\t; store expression result into rax");
			Emit(out, context, "");

			Emit(out, context, "    lea rcx, [rel builtin_print_format]\t\t; put 1st argument into rcx");
			Emit(out, context, "    mov rdx, rax\t\t\t; put 2nd argument into rdx");
			PushShadowSpace(out, context);
			Emit(out, context, "    call printf");
			PopShadowSpace(out, context);
			Emit(out, context, "");
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

				Emit(out, context, "    pop rax\t\t\t; store expression result into rax");
			}
			else
			{
				// bare return;
			}

			Emit(out, context, "    jmp .epilogue\t\t; return");
			Emit(out, context, "");
		} break;

		case NodeKind_Asm:
		{
			AsmNode *node = As<AsmNode>(baseNode);

			Emit(out, context, "    ; inline assembly begin");
			Emit(out, context, STR_FMT, STR_ARG(node->code));
			Emit(out, context, "");
			Emit(out, context, "    ; inline assembly end");
			Emit(out, context, "");
		} break;

		case NodeKind_Yield:
		{
			YieldNode *node = As<YieldNode>(baseNode);

			Emit(out, context, "    mov rax, [rbp - 8]");
			Emit(out, context, "    mov [rax], %d", node->yieldIndex);
			Emit(out, context, "    jmp .epilogue");
			Emit(out, context, ".coroutine_state_%d:", node->yieldIndex);
			Emit(out, context, "");
		} break;

		default:
		{
			GenerateExpression(baseNode, out, context);

			Emit(out, context, "    pop rax\t\t\t; discard the result");
			Emit(out, context, "");
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
			if (node->isForeign)
			{
				break;
			}

			BlockNode *functionBody = As<BlockNode>(node->body);

			Emit(out, context, STR_FMT ":", STR_ARG(node->name));
			Emit(out, context, "    push rbp");
			context->stackDepth--;
			Emit(out, context, "    mov rbp, rsp");
			Emit(out, context, "    sub rsp, %d", functionBody->stackSize);
			Emit(out, context, "");

			const char *paramRegs[] =
			{
				"rcx",
				"rdx",
				"r8",
				"r9",
			};

			int numParamsInRegs = Min(node->numParams, 4);

			for (int i = 0;
				 i < numParamsInRegs;
				 i++)
			{
				ParamNode *param = As<ParamNode>(node->params[i]);

				Emit(out, context, "    mov [rbp - %d], %s\t\t; unpack argument", param->stackOffset, paramRegs[i]);
			}

			for (int i = numParamsInRegs;
				 i < node->numParams;
				 i++)
			{
				ParamNode *param = As<ParamNode>(node->params[i]);

				int callerOffset = 48 + (i - numParamsInRegs)*8;
				Emit(out, context, "    mov rax, [rbp + %d]\t\t; unpack stack argument %d", callerOffset, i+1);
				Emit(out, context, "    mov [rbp - %d], rax", param->stackOffset);
			}
			Emit(out, context, "");

			if (node->isCoroutine)
			{
				Emit(out, context, "    mov rax, [rbp - 8]");
				Emit(out, context, "    mov rax, [rax]");
				Emit(out, context, "");

				for (int i = 0;
					 i <= node->yieldIndex;
					 i++)
				{
					Emit(out, context, "    cmp rax, %d", i);
					Emit(out, context, "    je .coroutine_state_%d", i);
				}
				Emit(out, context, "");

				Emit(out, context, "    jmp .epilogue");
				Emit(out, context, "");

				Emit(out, context, ".coroutine_state_0:");
			}

			GenerateBlock(node->body, out, context);

			if (node->isCoroutine)
			{
				Emit(out, context, "    mov rax, qword [rbp - 8]");
				Emit(out, context, "    mov qword [rax], -1");
				Emit(out, context, "");
			}

			Emit(out, context, ".epilogue:");
			Emit(out, context, "    mov rsp, rbp");
			Emit(out, context, "    pop rbp");
			context->stackDepth++;
			Emit(out, context, "    ret");
			Emit(out, context, "");

			Assert(context->stackDepth == 0);
		} break;

		case NodeKind_StructDecl:
		case NodeKind_EnumDecl:
		case NodeKind_ConstantDecl:
		{
			// do nothing
		} break;

		case NodeKind_Asm:
		{
			AsmNode *node = As<AsmNode>(baseNode);

			fprintf(out, "; inline assembly begin\n");
			fprintf(out, STR_FMT, STR_ARG(node->code));
			fprintf(out, "\n");
			fprintf(out, "; inline assembly end\n");
			fprintf(out, "\n");
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

internal int
WriteStringBytes(string str,
				 FILE *out)
{
	int stringLength = 0;

	bool needComma = false;

	for (usize i = 0;
		 i < str.count;
		 i++)
	{
		if (needComma)
		{
			fprintf(out, ",");
		}

		if (str[i] == '\\')
		{
			i++;
			if (str[i] == 'n')
			{
				fprintf(out, "%d", '\n');
			}
			else if (str[i] == 't')
			{
				fprintf(out, "%d", '\t');
			}
			else if (str[i] == 'r')
			{
				fprintf(out, "%d", '\r');
			}
			else if (str[i] == '0')
			{
				fprintf(out, "%d", '\0');
			}
			else if (str[i] == '\\')
			{
				fprintf(out, "%d", '\\');
			}
			else if (str[i] == '"')
			{
				fprintf(out, "%d", '"');
			}
			else
			{
				Assert(false);
			}
		}
		else
		{
			fprintf(out, "%d", str[i]);
		}

		stringLength++;
		needComma = true;
	}

	return stringLength;
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

			string linkName = node->name;
			if (node->isForeign)
			{
				if (node->foreignLinkName.count > 0)
				{
					linkName = node->foreignLinkName;
				}
			}

			if (node->isForeign)
			{
				fprintf(out, "extern " STR_FMT "\n", STR_ARG(linkName));
			}
			else
			{
				fprintf(out, "global " STR_FMT "\n", STR_ARG(linkName));
			}
		}
	}
	fprintf(out, "\n");

	fprintf(out, "extern printf\n");
	fprintf(out, "\n");

	fprintf(out, "section .data\n");
	fprintf(out, "builtin_print_format: db \"%%d\", 10, 0\t\t; 10 is newline, 0 is null terminator\n");

	for (auto &literal : context->cstringLiterals)
	{
		fprintf(out, "cstring_literal_%d: db ", literal.uniqueLabelId);

		WriteStringBytes(literal.value, out);

		fprintf(out, ",0\n");
	}
	fprintf(out, "\n");

	for (auto &literal : context->stringLiterals)
	{
		fprintf(out, "string_literal_%d_bytes: db ", literal.uniqueLabelId);

		int stringLength = WriteStringBytes(literal.value, out);

		fprintf(out, ",0\n");

		fprintf(out, "string_literal_%d: dq string_literal_%d_bytes, %d\n",
				literal.uniqueLabelId,
				literal.uniqueLabelId,
				stringLength);
	}
	fprintf(out, "\n");

	fprintf(out, "section .text\n");

	for (Node *it : program->statements)
	{
		GenerateTopLevelStatement(it, out, context);
	}

	Assert(context->stackDepth == 0);
}
