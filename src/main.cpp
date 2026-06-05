#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"

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

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: compiler <filename>\n");
		return 1;
	}

	const char *fileName = argv[1];
	
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

	Parser parser = {};
	parser.current = GetToken(&lexer);
	ParseExpression(&parser, &lexer, 0);
}
