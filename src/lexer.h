#pragma once

#include "common.h"

enum TokenKind : u32
{
	TokenKind_EOF   = 0,
	TokenKind_Error = 1,

	/* single-character tokens */
	TokenKind_Bang         = '!',
	TokenKind_Hash         = '#',
	TokenKind_Percent      = '%',
	TokenKind_Ampersand    = '&',
	TokenKind_OpenParen    = '(',
	TokenKind_CloseParen   = ')',
	TokenKind_Star         = '*',
	TokenKind_Plus         = '+',
	TokenKind_Comma        = ',',
	TokenKind_Minus        = '-',
	TokenKind_Dot          = '.',
	TokenKind_Slash        = '/',
	TokenKind_Colon        = ':',
	TokenKind_Semicolon    = ';',
	TokenKind_Less         = '<',
	TokenKind_Equal        = '=',
	TokenKind_Greater      = '>',
	TokenKind_OpenBracket  = '[',
	TokenKind_CloseBracket = ']',
	TokenKind_Caret        = '^',
	TokenKind_OpenBrace    = '{',
	TokenKind_Pipe         = '|',
	TokenKind_CloseBrace   = '}',
	TokenKind_Tilde        = '~',

	/* two-character tokens */
	TokenKind_BangEqual = 128,
	TokenKind_EqualEqual,
	TokenKind_GreaterEqual,
	TokenKind_LessEqual,
	TokenKind_Arrow,
	TokenKind_AmpAmp,
	TokenKind_PipePipe,
	TokenKind_GreaterGreater,
	TokenKind_LessLess,
	TokenKind_PlusEqual,
	TokenKind_MinusEqual,
	TokenKind_PercentEqual,
	TokenKind_StarEqual,
	TokenKind_SlashEqual,
	TokenKind_DotDot,

	/* three-character tokens */
	TokenKind_DotDotLess,

	/* keywords */
	TokenKind_If,
	TokenKind_Else,
	TokenKind_While,
	TokenKind_Do,
	TokenKind_Print,
	TokenKind_Proc,
	TokenKind_Return,
	TokenKind_True,
	TokenKind_False,
	TokenKind_Struct,
	TokenKind_For,
	TokenKind_Asm,
	TokenKind_Enum,
	TokenKind_Cast,
	TokenKind_Yield,
	TokenKind_Break,
	TokenKind_Continue,
	TokenKind_In,
	TokenKind_Foreach,
	TokenKind_Defer,
	TokenKind_Switch,
	TokenKind_Case,
	TokenKind_Default,
	TokenKind_Null,
	TokenKind_Macro,
	TokenKind_Sizeof,

	/* other */
	TokenKind_Identifier,
	TokenKind_Int64Literal,
	TokenKind_Float32Literal,
	TokenKind_Float64Literal,
	TokenKind_String,
	TokenKind_CString,
};

inline char *
GetTokenKindName(TokenKind kind)
{
	switch (kind)
	{
		case TokenKind_EOF:            return "<eof>";
		case TokenKind_Error:          return "<error>";
		case TokenKind_Bang:           return "!";
		case TokenKind_Hash:           return "#";
		case TokenKind_Percent:        return "%";
		case TokenKind_Ampersand:      return "&";
		case TokenKind_OpenParen:      return "(";
		case TokenKind_CloseParen:     return ")";
		case TokenKind_Star:           return "*";
		case TokenKind_Plus:           return "+";
		case TokenKind_Comma:          return ",";
		case TokenKind_Minus:          return "-";
		case TokenKind_Dot:            return ".";
		case TokenKind_Slash:          return "/";
		case TokenKind_Colon:          return ":";
		case TokenKind_Semicolon:      return ";";
		case TokenKind_Less:           return "<";
		case TokenKind_Equal:          return "=";
		case TokenKind_Greater:        return ">";
		case TokenKind_OpenBracket:    return "[";
		case TokenKind_CloseBracket:   return "]";
		case TokenKind_Caret:          return "^";
		case TokenKind_OpenBrace:      return "{";
		case TokenKind_Pipe:           return "|";
		case TokenKind_CloseBrace:     return "}";
		case TokenKind_Tilde:          return "~";
		case TokenKind_BangEqual:      return "!=";
		case TokenKind_EqualEqual:     return "==";
		case TokenKind_GreaterEqual:   return ">=";
		case TokenKind_LessEqual:      return "<=";
		case TokenKind_Arrow:          return "->";
		case TokenKind_AmpAmp:         return "&&";
		case TokenKind_PipePipe:       return "||";
		case TokenKind_GreaterGreater: return ">>";
		case TokenKind_LessLess:       return "<<";
		case TokenKind_PlusEqual:      return "+=";
		case TokenKind_MinusEqual:     return "-=";
		case TokenKind_PercentEqual:   return "%=";
		case TokenKind_StarEqual:      return "*=";
		case TokenKind_SlashEqual:     return "/=";
		case TokenKind_DotDot:         return "..";
		case TokenKind_DotDotLess:     return "..<";
		case TokenKind_If:             return "if";
		case TokenKind_Else:           return "else";
		case TokenKind_While:          return "while";
		case TokenKind_Do:             return "do";
		case TokenKind_Print:          return "print";
		case TokenKind_Proc:           return "proc";
		case TokenKind_Return:         return "return";
		case TokenKind_True:           return "true";
		case TokenKind_False:          return "false";
		case TokenKind_Struct:         return "struct";
		case TokenKind_For:            return "for";
		case TokenKind_Asm:            return "asm";
		case TokenKind_Enum:           return "enum";
		case TokenKind_Cast:           return "cast";
		case TokenKind_Yield:          return "yield";
		case TokenKind_Break:          return "break";
		case TokenKind_Continue:       return "continue";
		case TokenKind_In:             return "in";
		case TokenKind_Foreach:        return "foreach";
		case TokenKind_Defer:          return "defer";
		case TokenKind_Switch:         return "switch";
		case TokenKind_Case:           return "case";
		case TokenKind_Default:        return "default";
		case TokenKind_Null:           return "null";
		case TokenKind_Macro:          return "macro";
		case TokenKind_Sizeof:         return "sizeof";
		case TokenKind_Identifier:     return "identifier";
		case TokenKind_Int64Literal:   return "number";
		case TokenKind_Float32Literal: return "number";
		case TokenKind_Float64Literal: return "number";
		case TokenKind_String:         return "string";
		case TokenKind_CString:        return "cstring";
	}

	Assert(false);
	return "<unknown>";
}

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
