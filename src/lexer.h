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
	/* three-character tokens */ \
	X(TokenKind_DotDotLess,       140,       "..<"            ) \
	/* keywords */ \
	X(TokenKind_If,               141,       "if"             ) \
	X(TokenKind_Else,             142,       "else"           ) \
	X(TokenKind_While,            143,       "while"          ) \
	X(TokenKind_Do,               144,       "do"             ) \
	X(TokenKind_Print,            145,       "print"          ) \
	X(TokenKind_Proc,             146,       "proc"           ) \
	X(TokenKind_Return,           147,       "return"         ) \
	X(TokenKind_True,             148,       "true"           ) \
	X(TokenKind_False,            149,       "false"          ) \
	X(TokenKind_Struct,           150,       "struct"         ) \
	X(TokenKind_For,              151,       "for"            ) \
	X(TokenKind_Asm,              152,       "asm"            ) \
	X(TokenKind_Enum,             153,       "enum"           ) \
	X(TokenKind_Cast,             154,       "cast"           ) \
	X(TokenKind_Yield,            155,       "yield"          ) \
	X(TokenKind_Break,            156,       "break"          ) \
	X(TokenKind_Continue,         157,       "continue"       ) \
	X(TokenKind_In,               158,       "in"             ) \
	X(TokenKind_Foreach,          159,       "foreach"        ) \
	/* other */ \
	X(TokenKind_Identifier,       160,       "identifier"     ) \
	X(TokenKind_Int64Literal,     161,       "number"         ) \
	X(TokenKind_Float32Literal,   162,       "number"         ) \
	X(TokenKind_Float64Literal,   163,       "number"         ) \
	X(TokenKind_String,           164,       "string"         ) \
	X(TokenKind_CString,          165,       "cstring"        )

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
	TokenKind kind;

	union
	{
		i64 int64Value;
		f32 float32Value;
		f64 float64Value;
	};
};

struct LexerFrame
{
	string fileName;
	char *current;
	char *lineStart;
	int line;
};

constexpr usize MAX_INCLUDE_DEPTH = 32;

struct LexerContext
{
	string compilerExeFileDir;
	string modulesDir;
};

struct Lexer
{
	string fileName;
	char *current;
	char *lineStart;
	int line;

	StaticBumpArray<LexerFrame, MAX_INCLUDE_DEPTH> includeStack;

	LexerContext *context;
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
