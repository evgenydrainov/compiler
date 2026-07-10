#pragma once

#include "common.h"

#define TOKEN_KIND_LIST(X) \
	X(TokenKind_EOF,              0,         "<eof>"          ) \
	X(TokenKind_Error,            1,         "<error>"        ) \
	/* single-character tokens */ \
	X(TokenKind_Bang,             '!',       "!"              ) \
	X(TokenKind_Hash,             '#',       "#"              ) \
	X(TokenKind_Percent,          '%',       "%"              ) \
	X(TokenKind_Ampersand,        '&',       "&"              ) \
	X(TokenKind_OpenParen,        '(',       "("              ) \
	X(TokenKind_CloseParen,       ')',       ")"              ) \
	X(TokenKind_Asterisk,         '*',       "*"              ) \
	X(TokenKind_Plus,             '+',       "+"              ) \
	X(TokenKind_Comma,            ',',       ","              ) \
	X(TokenKind_Minus,            '-',       "-"              ) \
	X(TokenKind_Dot,              '.',       "."              ) \
	X(TokenKind_Slash,            '/',       "/"              ) \
	X(TokenKind_Colon,            ':',       ":"              ) \
	X(TokenKind_Semicolon,        ';',       ";"              ) \
	X(TokenKind_Less,             '<',       "<"              ) \
	X(TokenKind_Equal,            '=',       "="              ) \
	X(TokenKind_Greater,          '>',       ">"              ) \
	X(TokenKind_OpenBracket,      '[',       "["              ) \
	X(TokenKind_CloseBracket,     ']',       "]"              ) \
	X(TokenKind_Caret,            '^',       "^"              ) \
	X(TokenKind_OpenBrace,        '{',       "{"              ) \
	X(TokenKind_Pipe,             '|',       "|"              ) \
	X(TokenKind_CloseBrace,       '}',       "}"              ) \
	X(TokenKind_Tilde,            '~',       "~"              ) \
	/* two-character tokens */ \
	X(TokenKind_BangEqual,        128,       "!="             ) \
	X(TokenKind_EqualEqual,       129,       "=="             ) \
	X(TokenKind_GreaterEqual,     130,       ">="             ) \
	X(TokenKind_LessEqual,        131,       "<="             ) \
	X(TokenKind_Arrow,            132,       "->"             ) \
	X(TokenKind_AmpAmp,           133,       "&&"             ) \
	X(TokenKind_PipePipe,         134,       "||"             ) \
	X(TokenKind_GreaterGreater,   135,       ">>"             ) \
	X(TokenKind_LessLess,         136,       "<<"             ) \
	X(TokenKind_PlusEqual,        137,       "+="             ) \
	X(TokenKind_MinusEqual,       138,       "-="             ) \
	X(TokenKind_PercentEqual,     139,       "%="             ) \
	/* keywords */ \
	X(TokenKind_If,               140,       "if"             ) \
	X(TokenKind_Else,             141,       "else"           ) \
	X(TokenKind_While,            142,       "while"          ) \
	X(TokenKind_Do,               143,       "do"             ) \
	X(TokenKind_Print,            144,       "print"          ) \
	X(TokenKind_Proc,             145,       "proc"           ) \
	X(TokenKind_Return,           146,       "return"         ) \
	X(TokenKind_True,             147,       "true"           ) \
	X(TokenKind_False,            148,       "false"          ) \
	X(TokenKind_Struct,           149,       "struct"         ) \
	X(TokenKind_For,              150,       "for"            ) \
	X(TokenKind_Asm,              151,       "asm"            ) \
	X(TokenKind_Enum,             152,       "enum"           ) \
	X(TokenKind_Cast,             153,       "cast"           ) \
	/* other */ \
	X(TokenKind_Identifier,       154,       "identifier"     ) \
	X(TokenKind_Number,           155,       "number"         ) \
	X(TokenKind_String,           156,       "string"         ) \
	X(TokenKind_CString,          157,       "cstring"        )

DEFINE_ENUM_WITH_VALUES(TokenKind, u32, TOKEN_KIND_LIST);

struct SourceLocation
{
	string fileName;
	int line;
	int column;
};

struct Token
{
	SourceLocation location;
	string str;
	i64 numberValue;
	TokenKind kind;
};

struct LexerFrame
{
	string fileName;
	char *current;
	char *lineStart;
	int line;
};

#define MAX_INCLUDE_DEPTH 32

struct Lexer
{
	string fileName;
	char *current;
	char *lineStart;
	int line;

	StaticBumpArray<LexerFrame, MAX_INCLUDE_DEPTH> includeStack;
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
