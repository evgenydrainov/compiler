#include "print_code.h"
#include "parser.h"

#include <stdio.h>
#include <stdarg.h>

internal void
Print(PrintContext *context,
	  char *format, ...)
{
	if (context->fromNewLine)
	{
		for (int i = 0; i < context->indentation; i++)
		{
			printf("    ");
		}
		context->fromNewLine = false;
	}

	va_list args;
	va_start(args, format);

	vprintf(format, args);

	va_end(args);
}

internal void
PrintLn(PrintContext *context,
		char *format, ...)
{
	if (context->fromNewLine)
	{
		for (int i = 0; i < context->indentation; i++)
		{
			printf("    ");
		}
		context->fromNewLine = false;
	}

	va_list args;
	va_start(args, format);

	vprintf(format, args);

	if (context->suppressNewLines)
	{
		printf(" ");
		context->suppressNewLines--;
	}
	else
	{
		printf("\n");
		context->fromNewLine = true;
	}

	va_end(args);
}

internal void
PrintExpression(PrintContext *context,
				Node *_node)
{
	if (!_node)
	{
		return;
	}

	switch (_node->kind)
	{
		default:
		{
			Assert(false);
		} break;

		case NodeKind_Int64Literal:
		{
			Int64LiteralNode *node = As<Int64LiteralNode>(_node);

			Print(context, "%lld", node->value);
		} break;

		case NodeKind_Float32Literal:
		{
			Float32LiteralNode *node = As<Float32LiteralNode>(_node);

			Print(context, "%ff32", node->value);
		} break;

		case NodeKind_Float64Literal:
		{
			Float64LiteralNode *node = As<Float64LiteralNode>(_node);

			Print(context, "%ff64", node->value);
		} break;

		case NodeKind_BoolLiteral:
		{
			BoolLiteralNode *node = As<BoolLiteralNode>(_node);

			Print(context, "%s", node->value ? "true" : "false");
		} break;

		case NodeKind_CString:
		{
			CStringNode *node = As<CStringNode>(_node);

			Print(context, "\"" STR_FMT "\"c", STR_ARG(node->value));
		} break;

		case NodeKind_ProcRef:
		{
			ProcRefNode *node = As<ProcRefNode>(_node);

			Print(context, STR_FMT, STR_ARG(node->linkName));
		} break;

		case NodeKind_NullLiteral:
		{
			Print(context, "null");
		} break;

		case NodeKind_Binary:
		{
			BinaryNode *node = As<BinaryNode>(_node);

			Print(context, "(");

			PrintExpression(context, node->lhs);

			Print(context, " %s ", GetBinaryOpSymbol(node->op));

			PrintExpression(context, node->rhs);

			Print(context, ")");
		} break;

		case NodeKind_Unary:
		{
			UnaryNode *node = As<UnaryNode>(_node);

			Print(context, "%s", GetUnaryOpSymbol(node->op));

			PrintExpression(context, node->expr);
		} break;

		case NodeKind_Var:
		{
			VarNode *node = As<VarNode>(_node);

			Print(context, STR_FMT, STR_ARG(node->name));
		} break;

		case NodeKind_FieldAccess:
		{
			FieldAccessNode *node = As<FieldAccessNode>(_node);

			PrintExpression(context, node->expr);

			Print(context, "." STR_FMT, STR_ARG(node->fieldName));
		} break;

		case NodeKind_ArrayIndexAccess:
		{
			ArrayIndexAccessNode *node = As<ArrayIndexAccessNode>(_node);

			PrintExpression(context, node->arrayExpr);

			Print(context, "[");

			PrintExpression(context, node->indexExpr);

			Print(context, "]");
		} break;

		case NodeKind_Call:
		{
			CallNode *node = As<CallNode>(_node);

			PrintExpression(context, node->callee);

			Print(context, "(");

			bool needComma = false;

			for (Node *it : node->arguments)
			{
				if (needComma)
				{
					Print(context, ", ");
				}

				PrintExpression(context, it);

				needComma = true;
			}

			Print(context, ")");
		} break;

		case NodeKind_AddressOf:
		{
			AddressOfNode *node = As<AddressOfNode>(_node);

			Print(context, "&");

			PrintExpression(context, node->what);
		} break;

		case NodeKind_Cast:
		{
			CastNode *node = As<CastNode>(_node);

			Print(context, "cast(" STR_FMT ")",
				  STR_ARG(TypeToString(node->targetType)));

			PrintExpression(context, node->what);
		} break;
	}
}

internal void
PrintStatement(PrintContext *context,
			   Node *_node)
{
	if (!_node)
	{
		return;
	}

	switch (_node->kind)
	{
		case NodeKind_Block:
		{
			BlockNode *node = As<BlockNode>(_node);

			PrintLn(context, "{");

			context->indentation++;

			for (Node *statement : node->statements)
			{
				PrintStatement(context, statement);
			}

			context->indentation--;

			PrintLn(context, "}");
		} break;

		case NodeKind_If:
		{
			IfNode *node = As<IfNode>(_node);

			Print(context, "if ");

			PrintExpression(context, node->condition);

			PrintLn(context, "");

			PrintStatement(context, node->thenBlock);

			if (node->elseBlock)
			{
				PrintLn(context, "else");

				PrintStatement(context, node->elseBlock);
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(_node);

			Print(context, "while ");

			PrintExpression(context, node->condition);

			PrintLn(context, "");

			PrintStatement(context, node->body);
		} break;

		case NodeKind_For:
		{
			ForNode *node = As<ForNode>(_node);

			Print(context, "for ");

			context->suppressNewLines++;
			PrintStatement(context, node->init);

			PrintExpression(context, node->cond);
			Print(context, "; ");

			PrintStatement(context, node->incr);

			PrintStatement(context, node->body);
		} break;

		case NodeKind_Return:
		{
			ReturnNode *node = As<ReturnNode>(_node);

			if (node->expr)
			{
				Print(context, "return ");
				PrintExpression(context, node->expr);
				PrintLn(context, ";");
			}
			else
			{
				PrintLn(context, "return;");
			}
		} break;

		case NodeKind_VarDecl:
		{
			VarDeclNode *node = As<VarDeclNode>(_node);

			Print(context, STR_FMT ": " STR_FMT,
				  STR_ARG(node->name),
				  STR_ARG(TypeToString(node->type)));

			if (node->expr)
			{
				Print(context, " = ");

				PrintExpression(context, node->expr);
			}

			PrintLn(context, ";");
		} break;

		case NodeKind_Assign:
		{
			AssignNode *node = As<AssignNode>(_node);

			PrintExpression(context, node->lhs);

			Print(context, " = ");

			PrintExpression(context, node->rhs);

			PrintLn(context, ";");
		} break;

		case NodeKind_Switch:
		{
			SwitchNode *node = As<SwitchNode>(_node);

			Print(context, "switch ");

			PrintExpression(context, node->expr);

			PrintLn(context, "");
			PrintLn(context, "{");
			context->indentation++;

			for (CaseNode *it : node->cases)
			{
				Print(context, "case ");

				PrintExpression(context, it->label);

				PrintLn(context, ":");

				PrintStatement(context, it->body);
			}

			if (node->defaultBody)
			{
				PrintLn(context, "default:");

				PrintStatement(context, node->defaultBody);
			}

			context->indentation--;
			PrintLn(context, "}");
		} break;

		default:
		{
			PrintExpression(context, _node);

			PrintLn(context, ";");
		} break;
	}
}

internal void
PrintTopLevelStatement(PrintContext *context,
					   Node *_node)
{
	switch (_node->kind)
	{
		case NodeKind_ProcDecl:
		{
			ProcDeclNode *node = As<ProcDeclNode>(_node);

			Print(context, STR_FMT " :: proc(", STR_ARG(node->name));

			bool needComma = false;

			for (ParamNode *param : node->params)
			{
				if (needComma)
				{
					Print(context, ", ");
				}

				Print(context, STR_FMT ": " STR_FMT,
					  STR_ARG(param->name),
					  STR_ARG(TypeToString(param->type)));

				needComma = true;
			}

			Print(context, ")");

			if (node->returnType.kind != TypeKind_Void)
			{
				Print(context, " -> " STR_FMT,
					  STR_ARG(TypeToString(node->returnType)));
			}

			PrintLn(context, "");
			
			PrintStatement(context, node->body);

			PrintLn(context, "");
		} break;

		default: {} break;
	}
}

void
PrintProgram(PrintContext *context,
			 Node *_program)
{
	BlockNode *program = As<BlockNode>(_program);

	for (Node *it : program->statements)
	{
		PrintTopLevelStatement(context, it);
	}
}
