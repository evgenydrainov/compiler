#include "codegen.h"
#include <stdio.h>
#include <stdarg.h>

internal void
Emit(CodegenContext *context,
	 char *format,
	 ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(context->out, format, args);
	fprintf(context->out, "\n");

	va_end(args);

	string formatStr = { format, StrLen(format) };
	formatStr = trim_left(formatStr);

	if (starts_with(formatStr, "push "))
	{
		context->stackDepth++;
	}

	if (starts_with(formatStr, "pop "))
	{
		context->stackDepth--;
	}
}

internal void
GenerateStatement(Node *node,
				  CodegenContext *context);

internal void
EmitDefers(CodegenContext *context,
		   usize floor)
{
	for (usize i = context->deferStack.count;
		 i-- > floor;)
	{
		Emit(context, "; deferred statement begin");

		GenerateStatement(context->deferStack[i], context);

		Emit(context, "; deferred statement end");
	}
}

internal void
GenerateBlock(Node *baseNode,
			  CodegenContext *context)
{
	BlockNode *node = As<BlockNode>(baseNode);

	usize marker = context->deferStack.count;

	for (Node *it : node->statements)
	{
		GenerateStatement(it, context);
	}

	EmitDefers(context, marker);
	context->deferStack.count = marker;
}

internal void
PushShadowSpace(CodegenContext *context)
{
	int numBytes = 32;
	if (context->stackDepth % 2 == 1)
	{
		numBytes += 8;
	}

	Emit(context, "    sub rsp, %d\t\t; push shadow space", numBytes);
}

internal void
PopShadowSpace(CodegenContext *context)
{
	int numBytes = 32;
	if (context->stackDepth % 2 == 1)
	{
		numBytes += 8;
	}

	Emit(context, "    add rsp, %d\t\t; pop shadow space", numBytes);
}

internal void
EmitConvertRaxToIntegerType(Type type,
							CodegenContext *context)
{
	Assert(IsInteger(type));

	int size = SizeOfType(type);

	if (IsSignedInteger(type))
	{
		if (size == 8)
		{
			// already a full register
		}
		else if (size == 4)
		{
			Emit(context, "    movsxd rax, eax");
		}
		else if (size == 2)
		{
			Emit(context, "    movsx rax, ax");
		}
		else if (size == 1)
		{
			Emit(context, "    movsx rax, al");
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
			// already a full register
		}
		else if (size == 4)
		{
			Emit(context, "    mov eax, eax");
		}
		else if (size == 2)
		{
			Emit(context, "    movzx rax, ax");
		}
		else if (size == 1)
		{
			Emit(context, "    movzx rax, al");
		}
		else
		{
			Assert(false);
		}
	}
}

internal void
GenerateExpression(Node *baseNode,
				   CodegenContext *context);

internal void
GenerateLValueAddress(Node *baseNode,
					  CodegenContext *context)
{
	switch (baseNode->kind)
	{
		case NodeKind_Var:
		{
			VarNode *node = As<VarNode>(baseNode);

			if (node->isGlobal)
			{
				Emit(context, "    lea rax, [rel " STR_FMT "]\t; load address of variable " STR_FMT_QUOTED,
					 STR_ARG(node->name),
					 STR_ARG(node->name));
			}
			else
			{
				Emit(context, "    lea rax, [rbp - %d]\t; load address of variable " STR_FMT_QUOTED,
					 node->stackOffset,
					 STR_ARG(node->name));
			}
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_Deref:
		{
			DerefNode *node = As<DerefNode>(baseNode);

			GenerateExpression(node->what, context);
		} break;

		case NodeKind_FieldAccess:
		{
			FieldAccessNode *node = As<FieldAccessNode>(baseNode);

			if (node->expr->inferredType.kind == TypeKind_Struct
				|| node->expr->inferredType.kind == TypeKind_Slice
				|| node->expr->inferredType.kind == TypeKind_DynamicArray)
			{
				GenerateLValueAddress(node->expr, context);
			}
			else if (node->expr->inferredType.kind == TypeKind_Pointer)
			{
				GenerateExpression(node->expr, context);
			}
			else
			{
				Assert(false);
			}

			Emit(context, "    pop rax");
			Emit(context, "    add rax, %d\t\t; add offset of field " STR_FMT_QUOTED,
				 node->fieldOffset,
				 STR_ARG(node->fieldName));
			Emit(context, "    push rax");
		} break;

		case NodeKind_String:
		{
			StringNode *node = As<StringNode>(baseNode);

			Emit(context, "    lea rax, [rel string_literal_%d]", node->uniqueId);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_ArrayIndexAccess:
		{
			ArrayIndexAccessNode *node = As<ArrayIndexAccessNode>(baseNode);

			int size = 0;

			if (node->arrayExpr->inferredType.kind == TypeKind_Pointer)
			{
				GenerateExpression(node->arrayExpr, context);

				size = SizeOfType(*node->arrayExpr->inferredType.pointee);
			}
			else if (node->arrayExpr->inferredType.kind == TypeKind_Array)
			{
				GenerateLValueAddress(node->arrayExpr, context);

				size = SizeOfType(*node->arrayExpr->inferredType.arrayElementType);
			}
			else if (node->arrayExpr->inferredType.kind == TypeKind_Slice
					 || node->arrayExpr->inferredType.kind == TypeKind_DynamicArray)
			{
				GenerateLValueAddress(node->arrayExpr, context);

				// load the first 8 bytes of the value, which is the 'data'
				// field of the slice or dynamic array
				Emit(context, "    pop rax");
				Emit(context, "    mov rax, qword [rax]");
				Emit(context, "    push rax");

				size = SizeOfType(*node->arrayExpr->inferredType.arrayElementType);
			}
			else
			{
				Assert(false);
			}

			GenerateExpression(node->indexExpr, context);

			Emit(context, "    pop rcx");
			Emit(context, "    pop rbx");

			if (size == 1
				|| size == 2
				|| size == 4
				|| size == 8)
			{
				Emit(context, "    lea rax, [rbx + rcx * %d]", size);
				Emit(context, "    push rax");
			}
			else
			{
				Emit(context, "    imul rcx, rcx, %d", size);
				Emit(context, "    add rbx, rcx");
				Emit(context, "    push rbx");
			}
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(baseNode);

			Assert(!IsRegisterSized(node->inferredType));

			// this will push rax, which is the address of the returned struct
			GenerateExpression(node, context);
		} break;

		default:
		{
			Assert(false);
		} break;
	}
}

internal void
GenerateBinaryExpression(Node *baseNode,
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
			GenerateExpression(node->lhs, context);
			GenerateExpression(node->rhs, context);

			Emit(context, "    pop rcx");
			Emit(context, "    pop rax");

			if (node->inferredType.kind == TypeKind_Float32)
			{
				Emit(context, "    movd xmm1, ecx");
				Emit(context, "    movd xmm0, eax");

				if (node->op == BinaryOp_Add)
				{
					Emit(context, "    addss xmm0, xmm1\t; perform addition");
				}
				else if (node->op == BinaryOp_Subtract)
				{
					Emit(context, "    subss xmm0, xmm1\t; perform subtraction");
				}
				else if (node->op == BinaryOp_Multiply)
				{
					Emit(context, "    mulss xmm0, xmm1\t; perform multiplication");
				}
				else if (node->op == BinaryOp_Divide)
				{
					Emit(context, "    divss xmm0, xmm1\t; perform division");
				}
				else
				{
					Assert(false);
				}

				Emit(context, "    movd eax, xmm0");
			}
			else if (node->inferredType.kind == TypeKind_Float64)
			{
				Emit(context, "    movq xmm1, rcx");
				Emit(context, "    movq xmm0, rax");

				if (node->op == BinaryOp_Add)
				{
					Emit(context, "    addsd xmm0, xmm1\t; perform addition");
				}
				else if (node->op == BinaryOp_Subtract)
				{
					Emit(context, "    subsd xmm0, xmm1\t; perform subtraction");
				}
				else if (node->op == BinaryOp_Multiply)
				{
					Emit(context, "    mulsd xmm0, xmm1\t; perform multiplication");
				}
				else if (node->op == BinaryOp_Divide)
				{
					Emit(context, "    divsd xmm0, xmm1\t; perform division");
				}
				else
				{
					Assert(false);
				}

				Emit(context, "    movq rax, xmm0");
			}
			else
			{
				if (node->op == BinaryOp_Add)
				{
					Emit(context, "    add rax, rcx\t; perform addition");
				}
				else if (node->op == BinaryOp_Subtract)
				{
					Emit(context, "    sub rax, rcx\t; perform subtraction");
				}
				else if (node->op == BinaryOp_Multiply)
				{
					Emit(context, "    imul rax, rcx\t; perform multiplication");
				}
				else if (node->op == BinaryOp_Divide)
				{
					Emit(context, "    cqo     \t\t;");

					if (IsSignedInteger(node->inferredType))
					{
						Emit(context, "    idiv rcx\t\t; perform division");
					}
					else if (IsUnsignedInteger(node->inferredType))
					{
						Emit(context, "    div rcx\t\t; perform division");
					}
					else
					{
						Assert(false);
					}
				}
				else if (node->op == BinaryOp_Modulo)
				{
					Emit(context, "    cqo");

					if (IsSignedInteger(node->inferredType))
					{
						Emit(context, "    idiv rcx\t\t; perform division");
					}
					else if (IsUnsignedInteger(node->inferredType))
					{
						Emit(context, "    div rcx\t\t; perform division");
					}
					else
					{
						Assert(false);
					}

					Emit(context, "    mov rax, rdx");
				}
				else if (node->op == BinaryOp_ShiftLeft)
				{
					Emit(context, "    shl rax, cl");
				}
				else if (node->op == BinaryOp_ShiftRight)
				{
					if (IsSignedInteger(node->inferredType))
					{
						Emit(context, "    sar rax, cl");
					}
					else if (IsUnsignedInteger(node->inferredType))
					{
						Emit(context, "    shr rax, cl");
					}
					else
					{
						Assert(false);
					}
				}
				else if (node->op == BinaryOp_BitAnd)
				{
					Emit(context, "    and rax, rcx");
				}
				else if (node->op == BinaryOp_BitOr)
				{
					Emit(context, "    or rax, rcx");
				}
				else if (node->op == BinaryOp_BitXor)
				{
					Emit(context, "    xor rax, rcx");
				}
				else
				{
					Assert(false);
				}
			}

			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case BinaryOp_Greater:
		case BinaryOp_Less:
		case BinaryOp_EqualEqual:
		case BinaryOp_GreaterEqual:
		case BinaryOp_LessEqual:
		case BinaryOp_NotEqual:
		{
			GenerateExpression(node->lhs, context);
			GenerateExpression(node->rhs, context);

			char *setccInstruction = "";

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
				else if (IsUnsignedInteger(node->lhs->inferredType)
						 || IsFloatingPoint(node->lhs->inferredType))
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

			Emit(context, "    pop rcx");
			Emit(context, "    pop rax");

			if (node->lhs->inferredType.kind == TypeKind_Float32)
			{
				Emit(context, "    movq xmm1, rcx");
				Emit(context, "    movq xmm0, rax");
				Emit(context, "    ucomiss xmm0, xmm1");
			}
			else if (node->lhs->inferredType.kind == TypeKind_Float64)
			{
				Emit(context, "    movq xmm1, rcx");
				Emit(context, "    movq xmm0, rax");
				Emit(context, "    ucomisd xmm0, xmm1");
			}
			else
			{
				Emit(context, "    cmp rax, rcx");
			}

			Emit(context, "    %s al", setccInstruction);
			Emit(context, "    movzx rax, al");
			Emit(context, "    push rax\t\t; push the comparison result");
			Emit(context, "");
		} break;

		case BinaryOp_LogicalAnd:
		{
			int uniqueId = ++context->uniqueLabelId;

			GenerateExpression(node->lhs, context);

			Emit(context, "    pop rax");
			Emit(context, "    cmp rax, 0");
			Emit(context, "    je .and_false_%d", uniqueId);
			Emit(context, "");

			GenerateExpression(node->rhs, context);

			Emit(context, "    pop rax");
			Emit(context, "    cmp rax, 0");
			Emit(context, "    je .and_false_%d", uniqueId);
			Emit(context, "");

			Emit(context, "    mov rax, 1");
			Emit(context, "    jmp .and_end_%d", uniqueId);
			Emit(context, ".and_false_%d:", uniqueId);
			Emit(context, "    mov rax, 0");
			Emit(context, ".and_end_%d:", uniqueId);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case BinaryOp_LogicalOr:
		{
			int uniqueId = ++context->uniqueLabelId;

			GenerateExpression(node->lhs, context);

			Emit(context, "    pop rax");
			Emit(context, "    cmp rax, 0");
			Emit(context, "    jne .or_true_%d", uniqueId);
			Emit(context, "");

			GenerateExpression(node->rhs, context);

			Emit(context, "    pop rax");
			Emit(context, "    cmp rax, 0");
			Emit(context, "    jne .or_true_%d", uniqueId);
			Emit(context, "");

			Emit(context, "    mov rax, 0");
			Emit(context, "    jmp .or_end_%d", uniqueId);
			Emit(context, ".or_true_%d:", uniqueId);
			Emit(context, "    mov rax, 1");
			Emit(context, ".or_end_%d:", uniqueId);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;
	}
}

internal void
EmitCopyBytes(CodegenContext *context,
			  char *destReg, char *srcReg, int size)
{
	int i = 0;

	while (i < align_downward(size, 8))
	{
		if (srcReg)
		{
			Emit(context, "    mov rax, qword [%s + %d]", srcReg, i);
			Emit(context, "    mov qword [%s + %d], rax", destReg, i);
		}
		else
		{
			Emit(context, "    mov qword [%s + %d], 0", destReg, i);
		}

		i += 8;
	}

	// handle the tail
	while (i < size)
	{
		if (srcReg)
		{
			Emit(context, "    mov al, byte [%s + %d]", srcReg, i);
			Emit(context, "    mov byte [%s + %d], al", destReg, i);
		}
		else
		{
			Emit(context, "    mov byte [%s + %d], 0", destReg, i);
		}

		i++;
	}
}

internal void
GenerateExpression(Node *baseNode,
				   CodegenContext *context)
{
	switch (baseNode->kind)
	{
		default:
		{
			Assert(false);
		} break;

		case NodeKind_Int64Literal:
		{
			Int64LiteralNode *node = As<Int64LiteralNode>(baseNode);

			Emit(context, "    mov rax, %lld\t\t; load int64 literal", node->value);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_Float32Literal:
		{
			Float32LiteralNode *node = As<Float32LiteralNode>(baseNode);

			u32 u32Value = *(u32 *)&node->value;

			Emit(context, "    mov rax, 0x%X\t\t; load float32 literal %f", u32Value, node->value);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_Float64Literal:
		{
			Float64LiteralNode *node = As<Float64LiteralNode>(baseNode);

			u64 u64Value = *(u64 *)&node->value;

			Emit(context, "    mov rax, 0x%llX\t\t; load float64 literal %f", u64Value, node->value);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_NullLiteral:
		{
			Emit(context, "    mov rax, 0\t\t; load 'null' literal");
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_CString:
		{
			CStringNode *node = As<CStringNode>(baseNode);

			Emit(context, "    lea rax, [rel cstring_literal_%d]\t; load string literal", node->uniqueId);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_Bool:
		{
			BoolNode *node = As<BoolNode>(baseNode);

			Emit(context, "    mov rax, %d\t\t; load boolean literal", (int)node->boolValue);
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_ProcRef:
		{
			ProcRefNode *node = As<ProcRefNode>(baseNode);

			char *prefix = "proc_";
			if (node->inferredType.procInfo->isForeign
				|| node->linkName == "main")
			{
				prefix = "";
			}

			Emit(context, "    lea rax, [%s" STR_FMT "]\t; load address of procedure " STR_FMT_QUOTED,
				 prefix,
				 STR_ARG(node->linkName),
				 STR_ARG(node->linkName));
			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_Binary:
		{
			GenerateBinaryExpression(baseNode, context);
		} break;

		case NodeKind_Unary:
		{
			UnaryNode *node = As<UnaryNode>(baseNode);

			GenerateExpression(node->expr, context);

			if (node->inferredType.kind == TypeKind_Float32)
			{
				if (node->op == UnaryOp_Negate)
				{
					Emit(context, "    pop rax");
					Emit(context, "    xor eax, 0x80000000\t; float32 negate");
					Emit(context, "    push rax");
				}
				else
				{
					Assert(false);
				}
			}
			else if (node->inferredType.kind == TypeKind_Float64)
			{
				if (node->op == UnaryOp_Negate)
				{
					Emit(context, "    pop rax");
					Emit(context, "    mov rcx, 0x8000000000000000");
					Emit(context, "    xor rax, rcx\t; float64 negate");
					Emit(context, "    push rax");
				}
				else
				{
					Assert(false);
				}
			}
			else
			{
				if (node->op == UnaryOp_Negate)
				{
					Emit(context, "    pop rax");
					Emit(context, "    neg rax");
					Emit(context, "    push rax");
				}
				else if (node->op == UnaryOp_LogicalNot)
				{
					Emit(context, "    pop rax");
					Emit(context, "    cmp rax, 0");
					Emit(context, "    sete al");
					Emit(context, "    movzx rax, al");
					Emit(context, "    push rax");
				}
				else if (node->op == UnaryOp_BitNegate)
				{
					Emit(context, "    pop rax");
					Emit(context, "    not rax");
					Emit(context, "    push rax");
				}
				else
				{
					Assert(false);
				}
			}

			Emit(context, "");
		} break;

		case NodeKind_Var:
		case NodeKind_Deref:
		case NodeKind_FieldAccess:
		case NodeKind_ArrayIndexAccess:
		{
			int size = SizeOfType(baseNode->inferredType);

			GenerateLValueAddress(baseNode, context);
			Emit(context, "    pop rax");

			if (IsSignedInteger(baseNode->inferredType))
			{
				if (size == 8)
				{
					Emit(context, "    mov rax, qword [rax]");
				}
				else if (size == 4)
				{
					Emit(context, "    movsxd rax, dword [rax]");
				}
				else if (size == 2)
				{
					Emit(context, "    movsx rax, word [rax]");
				}
				else if (size == 1)
				{
					Emit(context, "    movsx rax, byte [rax]");
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
					Emit(context, "    mov rax, qword [rax]");
				}
				else if (size == 4)
				{
					Emit(context, "    mov eax, dword [rax]");
				}
				else if (size == 2)
				{
					Emit(context, "    movzx rax, word [rax]");
				}
				else if (size == 1)
				{
					Emit(context, "    movzx rax, byte [rax]");
				}
				else
				{
					Assert(false);
				}
			}

			Emit(context, "    push rax");
			Emit(context, "");
		} break;

		case NodeKind_Cast:
		{
			CastNode *node = As<CastNode>(baseNode);

			GenerateExpression(node->what, context);

			Type from = node->what->inferredType;
			Type to = node->targetType;

			bool fromFloat = IsFloatingPoint(from);
			bool toFloat = IsFloatingPoint(to);

			if (fromFloat
				|| toFloat
				|| (IsInteger(to) && SizeOfType(to) < 8))
			{
				Emit(context, "    pop rax");

				// widen the source
				if (from.kind == TypeKind_Float32)
				{
					Emit(context, "    movd xmm0, eax");
					Emit(context, "    cvtss2sd xmm0, xmm0");
				}
				else if (from.kind == TypeKind_Float64)
				{
					Emit(context, "    movq xmm0, rax");
				}

				// convert
				if (!fromFloat && toFloat)
				{
					Emit(context, "    cvtsi2sd xmm0, rax");
				}
				else if (fromFloat && !toFloat)
				{
					Emit(context, "    cvttsd2si rax, xmm0");
				}

				// narrow to the target
				if (to.kind == TypeKind_Float32)
				{
					Emit(context, "    cvtsd2ss xmm0, xmm0");
					Emit(context, "    movd eax, xmm0");
				}
				else if (to.kind == TypeKind_Float64)
				{
					Emit(context, "    movq rax, xmm0");
				}
				else if (IsInteger(to))
				{
					EmitConvertRaxToIntegerType(to, context);
				}

				Emit(context, "    push rax");
			}
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(baseNode);

			char *paramRegs[] =
			{
				"rcx",
				"rdx",
				"r8",
				"r9",
			};

			bool hiddenReturnArg = !IsRegisterSized(node->inferredType);

			int numArguments = (int)node->arguments.count;
			if (hiddenReturnArg)
			{
				numArguments++;
			}

			int numArgumentsInRegs = Min(numArguments, 4);
			int numArgumentsOnStack = numArguments - numArgumentsInRegs;

			GenerateExpression(node->callee, context);

			Emit(context, "    pop rax");
			Emit(context, "    mov qword [rbp - %d], rax\t; save the call target",
				 node->calleeSlotOffset);

			int padding = ((context->stackDepth + numArgumentsOnStack) % 2) ? 8 : 0;
			if (padding)
			{
				Emit(context, "    sub rsp, %d\t\t; align stack", padding);
				context->stackDepth++;
			}

			auto GetExpr = [&](int i) -> Node *
			{
				if (hiddenReturnArg)
				{
					if (i == 0)
					{
						return nullptr;
					}
					else
					{
						return node->arguments[i - 1];
					}
				}
				else
				{
					return node->arguments[i];
				}
			};

			auto GetParamType = [&](int i) -> Type *
			{
				if (hiddenReturnArg)
				{
					if (i == 0)
					{
						return nullptr;
					}
					else
					{
						int index = i - 1;
						if (index < node->signature->params.count)
						{
							return &node->signature->params[index];
						}
						else
						{
							Assert(node->signature->isVariadic);
							return &node->arguments[index]->inferredType;
						}
					}
				}
				else
				{
					int index = i;
					if (index < node->signature->params.count)
					{
						return &node->signature->params[index];
					}
					else
					{
						Assert(node->signature->isVariadic);
						return &node->arguments[index]->inferredType;
					}
				}
			};

			for (int i = numArguments;
				 i--;)
			{
				Node *expr = GetExpr(i);

				Type *paramType = GetParamType(i);

				if (expr)
				{
					if (IsRegisterSized(*paramType))
					{
						GenerateExpression(expr, context);
					}
					else
					{
						Emit(context, "    lea rdi, [rbp - %d]", expr->paramCopyOffset);

						GenerateLValueAddress(expr, context);
						Emit(context, "    pop rsi");

						EmitCopyBytes(context, "rdi", "rsi", SizeOfType(*paramType));

						Emit(context, "    push rdi");
					}
				}
				else
				{
					// pass the first argument: it is the address of the
					// function's return value
					Emit(context, "    lea rax, [rbp - %d]", node->returnSlotOffset);
					Emit(context, "    push rax");
				}
						
				/*
				if (function->isVariadic)
				{
					if (i >= 4)
					{
						if (node->expressions[i]->inferredType.kind == TypeKind_Float32)
						{
							// TODO: probably have to do promotion here?
							Assert(false);
						}
					}
				}
				*/
			}

			for (int i = 0;
				 i < numArgumentsInRegs;
				 i++)
			{
				Type *paramType = GetParamType(i);

				if (paramType && paramType->kind == TypeKind_Float32)
				{
					Emit(context, "    pop rax\t\t\t; put argument");
					Emit(context, "    movd xmm%d, eax", i);

					if (node->signature->isVariadic)
					{
						// do promotion
						Emit(context, "    cvtss2sd xmm%d, xmm%d", i, i);

						// also copy to regular register
						Emit(context, "    movq %s, xmm%d", paramRegs[i], i);
					}
				}
				else if (paramType && paramType->kind == TypeKind_Float64)
				{
					Emit(context, "    pop rax\t\t\t; put argument");
					Emit(context, "    movq xmm%d, rax", i);

					if (node->signature->isVariadic)
					{
						// also copy to regular register
						Emit(context, "    movq %s, xmm%d", paramRegs[i], i);
					}
				}
				else
				{
					Emit(context, "    pop %s\t\t\t; put argument", paramRegs[i]);
				}
			}
			Emit(context, "");

			Emit(context, "    sub rsp, 32\t\t; reserve shadow space");

			Emit(context, "    call qword [rbp - %d]", node->calleeSlotOffset);

			Emit(context, "    add rsp, 32\t\t; free shadow space");

			if (numArgumentsOnStack + padding/8 > 0)
			{
				int cleanup = numArgumentsOnStack*8 + padding;
				Emit(context, "    add rsp, %d\t\t; free stack arguments", cleanup);
				context->stackDepth -= numArgumentsOnStack + padding/8;
			}

			if (node->inferredType.kind == TypeKind_Float32)
			{
				Emit(context, "    movd eax, xmm0");
			}
			else if (node->inferredType.kind == TypeKind_Float64)
			{
				Emit(context, "    movq rax, xmm0");
			}

			Emit(context, "    push rax\t\t; push the function return value");
			Emit(context, "");
		} break;

		case NodeKind_AddressOf:
		{
			AddressOfNode *node = As<AddressOfNode>(baseNode);

			GenerateLValueAddress(node->what, context);
		} break;

		case NodeKind_Proxy:
		{
			ProxyNode *node = As<ProxyNode>(baseNode);

			GenerateExpression(node->proxy, context);
		} break;
	}
}

internal char *
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
	return "error";
}

internal void
GenerateStatement(Node *baseNode,
				  CodegenContext *context)
{
	switch (baseNode->kind)
	{
		default:
		{
			GenerateExpression(baseNode, context);

			Emit(context, "    pop rax\t\t\t; discard the result");
			Emit(context, "");
		} break;

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

				GenerateStatement(&assignNode, context);
			}
			else
			{
				// initialize to zero by default

				if (IsRegisterSized(varNode.inferredType))
				{
					GenerateLValueAddress(&varNode, context);

					Emit(context, "    pop rcx");
					Emit(context, "    mov rax, 0");

					char *reg = GetRegisterForTypeSize(varNode.inferredType);

					Emit(context, "    mov [rcx], %s", reg);
					Emit(context, "");
				}
				else
				{
					// TODO: maybe don't take this path for sizes 3/5/6/7

					GenerateLValueAddress(&varNode, context);

					Emit(context, "    pop rdi");

					EmitCopyBytes(context, "rdi", nullptr, SizeOfType(varNode.inferredType));

					Emit(context, "");
				}
			}
		} break;

		case NodeKind_Assign:
		{
			AssignNode *node = As<AssignNode>(baseNode);

			if (IsRegisterSized(node->lhs->inferredType))
			{
				GenerateExpression(node->rhs, context);
				GenerateLValueAddress(node->lhs, context);

				Emit(context, "    pop rcx");
				Emit(context, "    pop rax");

				char *reg = GetRegisterForTypeSize(node->lhs->inferredType);

				Emit(context, "    mov [rcx], %s", reg);
				Emit(context, "");
			}
			else
			{
				// TODO: maybe don't take this path for sizes 3/5/6/7

				Assert(SizeOfType(node->lhs->inferredType) == SizeOfType(node->rhs->inferredType));

				GenerateLValueAddress(node->rhs, context);
				GenerateLValueAddress(node->lhs, context);

				Emit(context, "    pop rdi");
				Emit(context, "    pop rsi");

				EmitCopyBytes(context, "rdi", "rsi", SizeOfType(node->lhs->inferredType));

				Emit(context, "");
			}
		} break;

		case NodeKind_If:
		{
			IfNode *node = As<IfNode>(baseNode);

			int uniqueId = ++context->uniqueLabelId;

			if (node->elseBlock)
			{
				GenerateExpression(node->condition, context);

				Emit(context, "    pop rax\t\t; load the comparison result");
				Emit(context, "    cmp rax, 0");
				Emit(context, "    je .else_%d", uniqueId);
				Emit(context, "");

				GenerateStatement(node->thenBlock, context);

				Emit(context, "    jmp .end_%d", uniqueId);
				Emit(context, ".else_%d:", uniqueId);

				GenerateStatement(node->elseBlock, context);

				Emit(context, ".end_%d:", uniqueId);
				Emit(context, "");
			}
			else
			{
				GenerateExpression(node->condition, context);

				Emit(context, "    pop rax\t\t; load the comparison result");
				Emit(context, "    cmp rax, 0");
				Emit(context, "    je .end_%d", uniqueId);
				Emit(context, "");

				GenerateStatement(node->thenBlock, context);

				Emit(context, ".end_%d:", uniqueId);
				Emit(context, "");
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);

			int uniqueId = ++context->uniqueLabelId;

			int saveCurrentLoopUniqueId = context->currentLoopUniqueId;
			usize saveCurrentLoopDeferFloor = context->currentLoopDeferFloor;

			context->currentLoopUniqueId = uniqueId;
			context->currentLoopDeferFloor = context->deferStack.count;

			Emit(context, ".loop_%d:", uniqueId);

			GenerateExpression(node->condition, context);

			Emit(context, "    pop rax\t\t; load the comparison result");
			Emit(context, "    cmp rax, 0");
			Emit(context, "    je .end_%d", uniqueId);
			Emit(context, "");

			GenerateBlock(node->body, context);

			Emit(context, ".continue_%d:", uniqueId);
			Emit(context, "    jmp .loop_%d", uniqueId);
			Emit(context, ".end_%d:", uniqueId);
			Emit(context, "");

			context->currentLoopUniqueId = saveCurrentLoopUniqueId;
			context->currentLoopDeferFloor = saveCurrentLoopDeferFloor;
		} break;

		case NodeKind_For:
		{
			ForNode *node = As<ForNode>(baseNode);

			int uniqueId = ++context->uniqueLabelId;

			int saveCurrentLoopUniqueId = context->currentLoopUniqueId;
			usize saveCurrentLoopDeferFloor = context->currentLoopDeferFloor;

			context->currentLoopUniqueId = uniqueId;
			context->currentLoopDeferFloor = context->deferStack.count;

			GenerateStatement(node->init, context);

			Emit(context, ".loop_%d:", uniqueId);

			GenerateExpression(node->cond, context);

			Emit(context, "    pop rax\t\t; load the comparison result");
			Emit(context, "    cmp rax, 0");
			Emit(context, "    je .end_%d", uniqueId);
			Emit(context, "");

			GenerateBlock(node->body, context);

			Emit(context, ".continue_%d:", uniqueId);
			GenerateStatement(node->incr, context);

			Emit(context, "    jmp .loop_%d", uniqueId);
			Emit(context, ".end_%d:", uniqueId);
			Emit(context, "");

			context->currentLoopUniqueId = saveCurrentLoopUniqueId;
			context->currentLoopDeferFloor = saveCurrentLoopDeferFloor;
		} break;

		case NodeKind_Print:
		{
			PrintNode *node = As<PrintNode>(baseNode);

			GenerateExpression(node->expr, context);

			Emit(context, "    pop rax\t\t\t; store expression result into rax");
			Emit(context, "");

			Emit(context, "    lea rcx, [rel builtin_print_format]\t\t; put 1st argument into rcx");
			Emit(context, "    mov rdx, rax\t\t\t; put 2nd argument into rdx");
			PushShadowSpace(context);
			Emit(context, "    call printf");
			PopShadowSpace(context);
			Emit(context, "");
		} break;

		case NodeKind_Block:
		{
			BlockNode *node = As<BlockNode>(baseNode);

			GenerateBlock(node, context);
		} break;

		case NodeKind_Return:
		{
			ReturnNode *node = As<ReturnNode>(baseNode);

			if (node->expr)
			{
				if (IsRegisterSized(node->expr->inferredType))
				{
					GenerateExpression(node->expr, context);

					EmitDefers(context, 0);

					Emit(context, "    pop rax\t\t\t; store expression result into rax");

					if (context->currentReturnType.kind == TypeKind_Float32)
					{
						Emit(context, "    movd xmm0, eax");
					}
					else if (context->currentReturnType.kind == TypeKind_Float64)
					{
						Emit(context, "    movq xmm0, rax");
					}
				}
				else
				{
					// push the address of the struct
					GenerateLValueAddress(node->expr, context);

					Emit(context, "    pop rsi");

					Emit(context, "    mov rdi, [rbp - 8]"); // [rbp - 8] is the first argument

					EmitCopyBytes(context, "rdi", "rsi", SizeOfType(node->expr->inferredType));

					EmitDefers(context, 0);

					Emit(context, "    mov rax, [rbp - 8]"); // ABI: also return the pointer
				}
			}
			else
			{
				// bare return;

				EmitDefers(context, 0);
			}

			Emit(context, "    jmp .epilogue\t\t; return");
			Emit(context, "");
		} break;

		case NodeKind_Asm:
		{
			AsmNode *node = As<AsmNode>(baseNode);

			Emit(context, "    ; inline assembly begin");
			Emit(context, STR_FMT, STR_ARG(node->code));
			Emit(context, "");
			Emit(context, "    ; inline assembly end");
			Emit(context, "");
		} break;

		case NodeKind_Yield:
		{
			YieldNode *node = As<YieldNode>(baseNode);

			Emit(context, "    mov rax, [rbp - 8]");
			Emit(context, "    mov qword [rax], %d", node->yieldIndex);
			Emit(context, "    jmp .epilogue");
			Emit(context, ".coroutine_state_%d:", node->yieldIndex);
			Emit(context, "");
		} break;

		case NodeKind_Break:
		{
			Assert(context->currentLoopUniqueId != 0);

			EmitDefers(context, context->currentLoopDeferFloor);

			Emit(context, "    jmp .end_%d\t; 'break'", context->currentLoopUniqueId);
			Emit(context, "");
		} break;

		case NodeKind_Continue:
		{
			Assert(context->currentLoopUniqueId != 0);

			EmitDefers(context, context->currentLoopDeferFloor);

			Emit(context, "    jmp .continue_%d\t; 'continue'", context->currentLoopUniqueId);
			Emit(context, "");
		} break;

		case NodeKind_Defer:
		{
			DeferNode *node = As<DeferNode>(baseNode);

			array_add(&context->deferStack, node->what);
		} break;

		case NodeKind_Switch:
		{
			SwitchNode *node = As<SwitchNode>(baseNode);

			int uniqueId = ++context->uniqueLabelId;

			GenerateExpression(node->expr, context);

			Emit(context, "    pop rax");

			{
				int i = 0;
				for (CaseNode *caseNode : node->cases)
				{
					Emit(context, "    cmp rax, %lld", caseNode->labelValue);
					Emit(context, "    je .case_%d_%d", uniqueId, i);

					i++;
				}
			}

			if (node->defaultBody)
			{
				Emit(context, "    jmp .default_%d", uniqueId);
			}
			else
			{
				Emit(context, "    jmp .end_%d", uniqueId);
			}

			{
				int i = 0;
				for (CaseNode *caseNode : node->cases)
				{
					Emit(context, ".case_%d_%d:", uniqueId, i);
					GenerateStatement(caseNode->body, context);
					Emit(context, "    jmp .end_%d", uniqueId);

					i++;
				}
			}

			if (node->defaultBody)
			{
				Emit(context, ".default_%d:", uniqueId);
				GenerateStatement(node->defaultBody, context);
				Emit(context, "    jmp .end_%d", uniqueId);
			}

			Emit(context, ".end_%d:", uniqueId);
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

			if (!node->body)
			{
				break;
			}

			BlockNode *functionBody = As<BlockNode>(node->body);

			context->currentReturnType = node->returnType;

			char *prefix = "proc_";
			if (node->isForeign
				|| node->linkName == "main")
			{
				prefix = "";
			}

			Emit(context, "%s" STR_FMT ":", prefix, STR_ARG(node->linkName));

			// doesn't affect stack depth
			Emit(context, "    push rbp");
			context->stackDepth--;

			Emit(context, "    mov rbp, rsp");
			Emit(context, "    sub rsp, %d", functionBody->stackSize);
			Emit(context, "");

			char *paramRegs[] =
			{
				"rcx",
				"rdx",
				"r8",
				"r9",
			};

			bool hiddenReturnArg = !IsRegisterSized(node->returnType);

			int numParams = (int)node->params.count;
			if (hiddenReturnArg)
			{
				numParams++;
			}

			int numParamsInRegs = Min(numParams, 4);

			auto GetParam = [&](int i) -> ParamNode *
			{
				if (hiddenReturnArg)
				{
					if (i == 0)
					{
						return nullptr;
					}
					else
					{
						return As<ParamNode>(node->params[i - 1]);
					}
				}
				else
				{
					return As<ParamNode>(node->params[i]);
				}
			};

			for (int i = 0;
				 i < numParamsInRegs;
				 i++)
			{
				ParamNode *param = GetParam(i);

				if (param)
				{
					if (IsRegisterSized(param->type))
					{
						if (param->type.kind == TypeKind_Float32)
						{
							Emit(context, "    movss [rbp - %d], xmm%d\t\t; unpack argument", param->stackOffset, i);
						}
						else if (param->type.kind == TypeKind_Float64)
						{
							Emit(context, "    movsd [rbp - %d], xmm%d\t\t; unpack argument", param->stackOffset, i);
						}
						else
						{
							Emit(context, "    mov [rbp - %d], %s\t\t; unpack argument", param->stackOffset, paramRegs[i]);
						}
					}
					else
					{
						// this argument is passed by reference

						Emit(context, "    lea rdi, [rbp - %d]", param->stackOffset);

						Emit(context, "    mov rsi, %s", paramRegs[i]);

						EmitCopyBytes(context, "rdi", "rsi", SizeOfType(param->type));
					}
				}
				else
				{
					// unpack the first argument, which is the pointer of this
					// function's return value
					Emit(context, "    mov [rbp - 8], %s", paramRegs[i]);
				}
			}

			for (int i = numParamsInRegs;
				 i < numParams;
				 i++)
			{
				ParamNode *param = GetParam(i);

				Assert(param);

				int callerOffset = 48 + (i - numParamsInRegs)*8;

				if (IsRegisterSized(param->type))
				{
					Emit(context, "    mov rax, [rbp + %d]\t\t; unpack stack argument %d", callerOffset, i+1);
					Emit(context, "    mov [rbp - %d], rax", param->stackOffset);
				}
				else
				{
					// this argument is passed by reference

					Emit(context, "    lea rdi, [rbp - %d]", param->stackOffset);

					Emit(context, "    mov rsi, [rbp + %d]", callerOffset);

					EmitCopyBytes(context, "rdi", "rsi", SizeOfType(param->type));
				}
			}
			Emit(context, "");

			if (node->isCoroutine)
			{
				Emit(context, "    mov rax, [rbp - 8]");
				Emit(context, "    mov rax, [rax]");
				Emit(context, "");

				for (int i = 0;
					 i <= node->yieldIndex;
					 i++)
				{
					Emit(context, "    cmp rax, %d", i);
					Emit(context, "    je .coroutine_state_%d", i);
				}
				Emit(context, "");

				Emit(context, "    jmp .epilogue");
				Emit(context, "");

				Emit(context, ".coroutine_state_0:");
			}

			GenerateBlock(node->body, context);

			if (node->isCoroutine)
			{
				Emit(context, "    mov rax, qword [rbp - 8]");
				Emit(context, "    mov qword [rax], -1");
				Emit(context, "");
			}

			Emit(context, ".epilogue:");
			Emit(context, "    mov rsp, rbp");

			// doesn't affect stack depth
			Emit(context, "    pop rbp");
			context->stackDepth++;

			Emit(context, "    ret");
			Emit(context, "");

			Assert(context->stackDepth == 0);

			Assert(context->deferStack.count == 0);
		} break;

		case NodeKind_StructDecl:
		case NodeKind_EnumDecl:
		case NodeKind_ConstantDecl:
		case NodeKind_VarDecl:
		case NodeKind_MacroDecl:
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
				CodegenContext *context)
{
	FILE *out = context->out;

	fprintf(out, "default rel\n");
	fprintf(out, "\n");

	BlockNode *program = As<BlockNode>(_program);

	for (Node *it : program->statements)
	{
		if (it->kind == NodeKind_Func)
		{
			FuncNode *node = As<FuncNode>(it);

			char *prefix = "proc_";
			if (node->isForeign
				|| node->linkName == "main")
			{
				prefix = "";
			}

			if (node->isForeign)
			{
				fprintf(out, "extern %s" STR_FMT "\n", prefix, STR_ARG(node->linkName));
			}
			else
			{
				fprintf(out, "global %s" STR_FMT "\n", prefix, STR_ARG(node->linkName));
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

	fprintf(out, "section .bss\n");
	for (Node *it : program->statements)
	{
		if (it->kind == NodeKind_VarDecl)
		{
			VarDeclNode *node = As<VarDeclNode>(it);

			int size = SizeOfType(node->type);

			fprintf(out, "    " STR_FMT " resb %d\n",
					STR_ARG(node->name),
					size);
		}
	}
	fprintf(out, "\n");

	fprintf(out, "section .text\n");
	for (Node *it : program->statements)
	{
		GenerateTopLevelStatement(it, out, context);
	}

	Assert(context->stackDepth == 0);
}
