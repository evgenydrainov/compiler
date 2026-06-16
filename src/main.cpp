#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

#if 0
internal void
PrintTree(AstNode *node,
		  const char *prefix,
		  bool isLeft)
{
	if (!node)
	{
		return;
	}

	printf("%s%s%s", prefix, isLeft ? "+-- " : "\\-- ", GetNodeTypeName(node->type));

	printf(" (line=%d)", node->line);

	if (node->type == NodeType_Number)
	{
		printf(" (%d)", node->number.value);
	}
	else if (node->type == NodeType_Assign)
	{
		printf(" (" STR_FMT ")", STR_ARG(node->assign.name));
	}
	else if (node->type == NodeType_VarDecl)
	{
		printf(" (" STR_FMT ")", STR_ARG(node->varDecl.name));
	}
	else if (node->type == NodeType_Var)
	{
		printf(" (" STR_FMT ")", STR_ARG(node->var.name));
	}
	else if (node->type == NodeType_Func)
	{
		printf(" (" STR_FMT ")", STR_ARG(node->func.name));
	}
	else if (node->type == NodeType_Call)
	{
		printf(" (" STR_FMT ")", STR_ARG(node->call.name));
	}

	printf("\n");

	char childPrefix[256];
	snprintf(childPrefix, sizeof(childPrefix), "%s%s", prefix, isLeft ? "|   " : "    ");

	switch (node->type)
	{
		case NodeType_Block:
		{
			for (int i = 0;
				 i < node->block.numStatements;
				 i++)
			{
				AstNode *statement = node->block.statements[i];

				PrintTree(statement, childPrefix, i != node->block.numStatements-1);
			}
		} break;

		case NodeType_If:
		{
			if (node->_if.elseBlock)
			{
				PrintTree(node->_if.condition, childPrefix, true);
				PrintTree(node->_if.thenBlock, childPrefix, true);
				PrintTree(node->_if.elseBlock, childPrefix, false);
			}
			else
			{
				PrintTree(node->_if.condition, childPrefix, true);
				PrintTree(node->_if.thenBlock, childPrefix, false);
			}
		} break;

		case NodeType_While:
		{
			PrintTree(node->_while.condition, childPrefix, true);
			PrintTree(node->_while.body, childPrefix, false);
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
			if (node->binary.lhs && node->binary.rhs)
			{
				PrintTree(node->binary.lhs, childPrefix, true);
				PrintTree(node->binary.rhs, childPrefix, false);
			}
			else if (node->binary.lhs)
			{
				PrintTree(node->binary.lhs, childPrefix, false);
			}
			else if (node->binary.rhs)
			{
				PrintTree(node->binary.rhs, childPrefix, false);
			}
		} break;

		case NodeType_Print:
		{
			PrintTree(node->print.expr, childPrefix, false);
		} break;

		case NodeType_VarDecl:
		{
			PrintTree(node->varDecl.expr, childPrefix, false);
		} break;

		case NodeType_Assign:
		{
			PrintTree(node->assign.expr, childPrefix, false);
		} break;

		case NodeType_Func:
		{
			PrintTree(node->func.body, childPrefix, false);
		} break;

		case NodeType_Return:
		{
			PrintTree(node->ret.expr, childPrefix, false);
		} break;

		case NodeType_Var:
		case NodeType_Number:
		case NodeType_Call:
		case NodeType_Bool:
		case NodeType_Param: {} break;
	}
}
#endif

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: compiler <filename>\n");
		return 1;
	}

	char outputFilePath[1024];
	strcpy_s(outputFilePath, argv[1]);

	{
		size_t i;

		bool found = false;

		for (i = strlen(outputFilePath);
			 i--;)
		{
			if (outputFilePath[i] == '.')
			{
				found = true;
				break;
			}
			else if (outputFilePath[i] == '/'
					 || outputFilePath[i] == '\\')
			{
				fprintf(stderr, "filename is invalid\n");
				exit(1);
			}
		}

		if (!found)
		{
			fprintf(stderr, "filename is invalid\n");
			exit(1);
		}

		/*outputFilePath[i++] = '.';
		outputFilePath[i++] = 'a';
		outputFilePath[i++] = 's';
		outputFilePath[i++] = 'm';*/
		outputFilePath[i++] = 0;
	}

	CompileOptions options = {};
	options.inputFilePath = argv[1];
	options.outputFilePath = outputFilePath;

	CompileResult result = Compile(options);

	return result;
}
