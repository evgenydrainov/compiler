#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic_pass.h"

internal string
LoadFile(const char *fileName)
{
	string result = {};
	
	FILE *file = fopen(fileName, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		usize fileSize = ftell(file);

		char *fileData = (char *)malloc(fileSize + 1);
		if (fileData)
		{
			fseek(file, 0, SEEK_SET);
			if (fread(fileData, 1, fileSize, file) == fileSize)
			{
				fileData[fileSize] = 0;
				result = {fileData, fileSize};
			}
		}
		
		fclose(file);
	}

	return result;
}

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
		case NodeType_Param: {} break;
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: compiler <filename>\n");
		return 1;
	}

	const char *fileName = argv[1];

	_chdir("test");
	
	string text = LoadFile(fileName);
	if (text.count == 0)
	{
		fprintf(stderr, "Couldn't load file '%s'\n", fileName);
		return 1;
	}

	Lexer lexer = {};
	lexer.line = 1;
	lexer.current = text.data;

#if 0
	Token token = GetToken(&lexer);
	while (token.type != TokenType_EOF)
	{
		printf("token: " STR_FMT "\t\ttype: %s\t\tline: %d\n",
			   STR_ARG(token.str),
			   GetTokenTypeName(token.type),
			   token.line);
		token = GetToken(&lexer);
	}
#endif

	Arena arena = {};
	arena.capacity = Megabytes(10);
	arena.data = (u8 *)malloc(arena.capacity);

	Parser parser = {};
	parser.current = GetToken(&lexer);

	AstNode *program = ParseProgram(&parser, &lexer, &arena);

	if (!program || parser.hadError)
	{
		fprintf(stderr, "parser failed\n");
		return 1;
	}

	SemanticContext semanticContext = {};
	SemanticPass(program, &semanticContext, &arena);

	if (!semanticContext.hadError)
	{
		PrintTree(program, "", false);

		CodegenContext codegenContext = {};

		FILE *out = fopen("test.asm", "wb");
		Generate_x86_64(program, out, &codegenContext);
		fclose(out);

		if (system("%USERPROFILE%\\AppData\\Local\\bin\\NASM\\nasm.exe -f win64 test.asm -o test.obj") == 0)
		{
			if (system("\"\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.44.35207\\bin\\Hostx64\\x64\\link.exe\" "
			   "/nologo test.obj msvcrt.lib legacy_stdio_definitions.lib "
			   "/LIBPATH:\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.44.35207\\lib\\x64\" "
			   "/LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\ucrt\\x64\" "
			   "/LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\um\\x64\"\"") == 0)
			{
				int code = system("test.exe");
				printf("return code: %d\n", code);
			}
			else
			{
				fprintf(stderr, "link failed\n");
				return 1;
			}
		}
		else
		{
			fprintf(stderr, "nasm failed\n");
			return 1;
		}
	}
	else
	{
		fprintf(stderr, "semantic error\n");
		return 1;
	}

	printf("Arena usage: %.2f%%\n", 100.0f*(arena.pos/(float)arena.capacity));
}
