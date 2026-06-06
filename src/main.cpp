#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"

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

	printf("%s%s%s", prefix, isLeft ? "+-- " : "|-- ", GetNodeTypeName(node->type));

	if (node->type == NodeType_Number)
	{
		printf(" (%d)", node->numberValue);
	}

	printf("\n");

	char childPrefix[256];
	snprintf(childPrefix, sizeof(childPrefix), "%s%s", prefix, isLeft ? "|   " : "    ");

	if (node->lhs && node->rhs)
	{
		PrintTree(node->lhs, childPrefix, true);
		PrintTree(node->rhs, childPrefix, false);
	}
	else if (node->lhs)
	{
		PrintTree(node->lhs, childPrefix, false);
	}
	else if (node->rhs)
	{
		PrintTree(node->rhs, childPrefix, false);
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
	AstNode *node = ParseExpression(&parser, &lexer, 0, &arena);

	PrintTree(node, "", false);

	FILE *out = fopen("test.asm", "wb");
	Generate_x86_64(node, out);
	fclose(out);

	system("%USERPROFILE%\\AppData\\Local\\bin\\NASM\\nasm.exe -f win64 test.asm -o test.obj");

	system("\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.44.35207\\bin\\Hostx64\\x64\\link.exe\" /nologo test.obj /subsystem:console /entry:main");
}
