#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define TOKEN_TYPE_LIST(X) \
	X(TokenType_EOF,          0) \
	X(TokenType_LeftParen,    '(') \
	X(TokenType_RightParen,   ')') \
	X(TokenType_LeftBrace,    '{') \
	X(TokenType_RightBrace,   '}') \
	X(TokenType_Comma,        ',') \
	X(TokenType_Dot,          '.') \
	X(TokenType_Minus,        '-') \
	X(TokenType_Plus,         '+') \
	X(TokenType_Semicolon,    ';') \
	X(TokenType_Slash,        '/') \
	X(TokenType_Asterisk,     '*') \
	X(TokenType_Bang,         '!') \
	X(TokenType_Equal,        '=') \
	X(TokenType_Greater,      '>') \
	X(TokenType_Less,         '<') \
	X(TokenType_LeftBracket,  '[') \
	X(TokenType_RightBracket, ']') \
	X(TokenType_Identifier,   256)

DEFINE_ENUM_WITH_VALUES(TokenType, u32, TOKEN_TYPE_LIST);

struct Token
{
	TokenType type;
	string str;
	int line;
};

struct Lexer
{
	char *current;
	int line;
};

internal bool
IsAlpha(char c)
{
	bool result = ((c >= 'a' && c <= 'z')
				   || (c >= 'A' && c <= 'Z')
				   || (c == '_'));
	return result;
}

internal bool
IsDigit(char c)
{
	bool result = (c >= '0' && c <= '9');
	return result;
}

internal bool
IsAtEnd(Lexer *lexer)
{
	bool result = (*lexer->current == 0);
	return result;
}

internal char
PeekChar(Lexer *lexer)
{
	char result = *lexer->current;
	return result;
}

internal char
PeekNextChar(Lexer *lexer)
{
	char result = 0;
	if (!IsAtEnd(lexer))
	{
		result = *(lexer->current + 1);
	}
	return result;
}

internal char
AdvanceChar(Lexer *lexer)
{
	char result = *lexer->current++;
	return result;
}

internal Token
MakeToken(Lexer *lexer, TokenType type, string str)
{
	Token result = {};
	result.type = type;
	result.str = str;
	result.line = lexer->line;

	return result;
}

internal Token
ParseIdentifier(Lexer *lexer)
{
	string str = {lexer->current, 0};

	while (IsAlpha(PeekChar(lexer))
		   || IsDigit(PeekChar(lexer)))
	{
		AdvanceChar(lexer);
		str.count++;
	}

	Token result = MakeToken(lexer, TokenType_Identifier, str);
	return result;
}

//internal Token
//ParseString(Lexer *lexer)
//{
//	// eat the opening quote
//	AdvanceChar(lexer);
//
//	while (!IsAtEnd(lexer) && PeekChar(lexer) != '"')
//	{
//
//	}
//}

internal void
SkipWhitespace(Lexer *lexer)
{
	bool done = false;
	while (!done)
	{
		char c = PeekChar(lexer);
		if (c == ' '
			|| c == '\t'
			|| c == '\r')
		{
			AdvanceChar(lexer);
		}
		else if (c == '\n')
		{
			lexer->line++;
			AdvanceChar(lexer);
		}
		else if (c == '/')
		{
			if (PeekNextChar(lexer) == '/')
			{
				while (!IsAtEnd(lexer) && PeekChar(lexer) != '\n')
				{
					AdvanceChar(lexer);
				}
			}
			else
			{
				done = true;
			}
		}
		else
		{
			done = true;
		}
	}
}

internal Token
GetToken(Lexer *lexer)
{
	SkipWhitespace(lexer);

	if (IsAtEnd(lexer))
	{
		return MakeToken(lexer, TokenType_EOF, {});
	}

	char c = PeekChar(lexer);

	if (IsAlpha(c))
	{
		return ParseIdentifier(lexer);
	}

	if (c == '('
		|| c == ')'
		|| c == '{'
		|| c == '}'
		|| c == ','
		|| c == '.'
		|| c == '-'
		|| c == '+'
		|| c == ';'
		|| c == '/'
		|| c == '*'
		|| c == '!'
		|| c == '='
		|| c == '<'
		|| c == '>'
		|| c == '['
		|| c == ']')
	{
		string str = {lexer->current, 1};
		AdvanceChar(lexer);
		return MakeToken(lexer, (TokenType)c, str);
	}

	if (c == '"')
	{
		//return ParseString(lexer);
	}

	return {};
}

internal string
LoadFile(const char *fileName)
{
	string result = {};
	
	FILE *file = fopen(fileName, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		usize fileSize = ftell(file);

		char *memory = (char *)malloc(fileSize + 1);
		if (memory)
		{
			fseek(file, 0, SEEK_SET);
			if (fread(memory, 1, fileSize, file) == fileSize)
			{
				memory[fileSize] = 0;
				result = {memory, fileSize};
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

	Token token = GetToken(&lexer);
	while (token.type != TokenType_EOF)
	{
		printf("token: " STR_FMT "\t\ttype: %s\n", STR_ARG(token.str), GetTokenTypeName(token.type));
		token = GetToken(&lexer);
	}
}
