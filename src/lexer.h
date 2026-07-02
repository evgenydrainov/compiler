#pragma once

#include "common.h"

#define TOKEN_KIND_LIST(X) \
	X(TokenKind_EOF,              0,         "<eof>"          ) \
	X(TokenKind_Error,            1,         "<error>"        ) \
	X(TokenKind_OpenParen,        '(',       "("              ) \
	X(TokenKind_CloseParen,       ')',       ")"              ) \
	X(TokenKind_OpenBrace,        '{',       "{"              ) \
	X(TokenKind_CloseBrace,       '}',       "}"              ) \
	X(TokenKind_Comma,            ',',       ","              ) \
	X(TokenKind_Dot,              '.',       "."              ) \
	X(TokenKind_Minus,            '-',       "-"              ) \
	X(TokenKind_Plus,             '+',       "+"              ) \
	X(TokenKind_Semicolon,        ';',       ";"              ) \
	X(TokenKind_Slash,            '/',       "/"              ) \
	X(TokenKind_Asterisk,         '*',       "*"              ) \
	X(TokenKind_Bang,             '!',       "!"              ) \
	X(TokenKind_Equal,            '=',       "="              ) \
	X(TokenKind_Greater,          '>',       ">"              ) \
	X(TokenKind_Less,             '<',       "<"              ) \
	X(TokenKind_OpenBracket,      '[',       "["              ) \
	X(TokenKind_CloseBracket,     ']',       "]"              ) \
	X(TokenKind_Colon,            ':',       ":"              ) \
	X(TokenKind_Ampersand,        '&',       "&"              ) \
	X(TokenKind_Percent,          '%',       "%"              ) \
	X(TokenKind_Hash,             '#',       "#"              ) \
	X(TokenKind_Pipe,             '|',       "|"              ) \
	X(TokenKind_Caret,            '^',       "^"              ) \
	X(TokenKind_Identifier,       256,       "identifier"     ) \
	X(TokenKind_String,           257,       "string"         ) \
	X(TokenKind_Number,           258,       "number"         ) \
	X(TokenKind_BangEqual,        259,       "!="             ) \
	X(TokenKind_EqualEqual,       260,       "=="             ) \
	X(TokenKind_GreaterEqual,     261,       ">="             ) \
	X(TokenKind_LessEqual,        262,       "<="             ) \
	X(TokenKind_Arrow,            263,       "->"             ) \
	X(TokenKind_If,               264,       "if"             ) \
	X(TokenKind_Else,             265,       "else"           ) \
	X(TokenKind_While,            266,       "while"          ) \
	X(TokenKind_Do,               267,       "do"             ) \
	X(TokenKind_Print,            268,       "print"          ) \
	X(TokenKind_Proc,             269,       "proc"           ) \
	X(TokenKind_Return,           270,       "return"         ) \
	X(TokenKind_True,             271,       "true"           ) \
	X(TokenKind_False,            272,       "false"          ) \
	X(TokenKind_Struct,           273,       "struct"         ) \
	X(TokenKind_For,              274,       "for"            ) \
	X(TokenKind_AmpAmp,           275,       "&&"             ) \
	X(TokenKind_PipePipe,         276,       "||"             ) \
	X(TokenKind_GreaterGreater,   277,       ">>"             ) \
	X(TokenKind_LessLess,         278,       "<<"             ) \
	X(TokenKind_CString,          279,       "cstring"        ) \
	X(TokenKind_Asm,              280,       "asm"            ) \
	X(TokenKind_PlusEqual,        281,       "+="             ) \
	X(TokenKind_MinusEqual,       282,       "-="             ) \
	X(TokenKind_PercentEqual,     283,       "%="             )

DEFINE_ENUM_WITH_VALUES(TokenKind, u32, TOKEN_KIND_LIST);

struct Token
{
	string str;
	i64 numberValue;
	TokenKind kind;
	int line;
};

struct Lexer
{
	char *current;
	int line;
};

Token GetToken(Lexer *lexer);

inline Token
PeekToken(Lexer *lexer, int count = 1)
{
	Lexer copyLexer = *lexer;
	Token result = {};
	while (count--)
	{
		result = GetToken(&copyLexer);
	}

	return result;
}
