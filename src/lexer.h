#pragma once

#include "common.h"

#define TOKEN_TYPE_LIST(X) \
	X(TokenType_EOF,              0,         "<eof>"          ) \
	X(TokenType_Error,            1,         "<error>"        ) \
	X(TokenType_OpenParen,        '(',       "("              ) \
	X(TokenType_CloseParen,       ')',       ")"              ) \
	X(TokenType_OpenBrace,        '{',       "{"              ) \
	X(TokenType_CloseBrace,       '}',       "}"              ) \
	X(TokenType_Comma,            ',',       ","              ) \
	X(TokenType_Dot,              '.',       "."              ) \
	X(TokenType_Minus,            '-',       "-"              ) \
	X(TokenType_Plus,             '+',       "+"              ) \
	X(TokenType_Semicolon,        ';',       ";"              ) \
	X(TokenType_Slash,            '/',       "/"              ) \
	X(TokenType_Asterisk,         '*',       "*"              ) \
	X(TokenType_Bang,             '!',       "!"              ) \
	X(TokenType_Equal,            '=',       "="              ) \
	X(TokenType_Greater,          '>',       ">"              ) \
	X(TokenType_Less,             '<',       "<"              ) \
	X(TokenType_OpenBracket,      '[',       "["              ) \
	X(TokenType_CloseBracket,     ']',       "]"              ) \
	X(TokenType_Colon,            ':',       ":"              ) \
	X(TokenType_Identifier,       256,       "identifier"     ) \
	X(TokenType_String,           257,       "string"         ) \
	X(TokenType_Number,           258,       "number"         ) \
	X(TokenType_BangEqual,        259,       "!="             ) \
	X(TokenType_EqualEqual,       260,       "=="             ) \
	X(TokenType_GreaterEqual,     261,       ">="             ) \
	X(TokenType_LessEqual,        262,       "<="             ) \
	X(TokenType_Arrow,            263,       "->"             ) \
	X(TokenType_If,               264,       "if"             ) \
	X(TokenType_Else,             265,       "else"           ) \
	X(TokenType_While,            266,       "while"          ) \
	X(TokenType_Do,               267,       "do"             ) \
	X(TokenType_Print,            268,       "print"          ) \
	X(TokenType_Proc,             269,       "proc"           ) \
	X(TokenType_Return,           270,       "return"         ) \
	X(TokenType_True,             271,       "true"           ) \
	X(TokenType_False,            272,       "false"          )

DEFINE_ENUM_WITH_VALUES(TokenType, u32, TOKEN_TYPE_LIST);

struct Token
{
	TokenType type;
	string str;
	int line;
	int numberValue;
};

struct Lexer
{
	char *current;
	int line;
};

Token GetToken(Lexer *lexer);

inline Token
PeekToken(Lexer *lexer)
{
	Lexer copyLexer = *lexer;

	Token result = GetToken(&copyLexer);

	return result;
}
