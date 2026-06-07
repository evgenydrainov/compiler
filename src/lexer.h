#pragma once

#include "common.h"

#define TOKEN_TYPE_LIST(X) \
	X(TokenType_EOF,          0) \
	X(TokenType_Error,        1) \
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
	X(TokenType_Colon,        ':') \
	X(TokenType_Identifier,   256) \
	X(TokenType_String,       257) \
	X(TokenType_Number,       258) \
	X(TokenType_BangEqual,    259) \
	X(TokenType_EqualEqual,   260) \
	X(TokenType_GreaterEqual, 261) \
	X(TokenType_LessEqual,    262) \
	X(TokenType_Arrow,        263) \
	X(TokenType_If,           264) \
	X(TokenType_Else,         265) \
	X(TokenType_While,        266) \
	X(TokenType_Do,           267)

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
