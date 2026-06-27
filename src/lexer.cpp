#include "lexer.h"

internal bool
IsAlpha(char c)
{
	bool result = ((c >= 'a' && c <= 'z')
				   || (c >= 'A' && c <= 'Z')
				   || (c == '_'));
	return result;
}

internal bool
IsHexadecimal(char c)
{
	bool result = ((c >= '0' && c <= '9')
				   || (c >= 'a' && c <= 'f')
				   || (c >= 'A' && c <= 'F'));
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

internal void
AdvanceChar(Lexer *lexer, int count = 1)
{
	lexer->current += count;
}

internal Token
MakeToken(Lexer *lexer, TokenKind kind, string str)
{
	Token result = {};
	result.kind = kind;
	result.str = str;
	result.line = lexer->line;

	return result;
}

internal Token
ErrorToken(Lexer *lexer, string message)
{
	Token result = {};
	result.kind = TokenKind_Error;
	result.str = message;
	result.line = lexer->line;

	return result;
}

internal TokenKind
IdentifierType(string str)
{
	TokenKind result = TokenKind_Identifier;
	
	Assert(str.count >= 1);

	switch (str[0])
	{
		case 'a':
		{
			if (str == "asm")
			{
				result = TokenKind_Asm;
			}
		} break;

		case 'i':
		{
			if (str == "if")
			{
				result = TokenKind_If;
			}
		} break;

		case 'e':
		{
			if (str == "else")
			{
				result = TokenKind_Else;
			}
		} break;

		case 'w':
		{
			if (str == "while")
			{
				result = TokenKind_While;
			}
		} break;

		case 'd':
		{
			if (str == "do")
			{
				result = TokenKind_Do;
			}
		} break;

		case 'p':
		{
			if (str == "print")
			{
				result = TokenKind_Print;
			}
			else if (str == "proc")
			{
				result = TokenKind_Proc;
			}
		} break;

		case 'r':
		{
			if (str == "return")
			{
				result = TokenKind_Return;
			}
		} break;

		case 't':
		{
			if (str == "true")
			{
				result = TokenKind_True;
			}
		} break;

		case 'f':
		{
			if (str == "false")
			{
				result = TokenKind_False;
			}
			else if (str == "for")
			{
				result = TokenKind_For;
			}
		} break;

		case 's':
		{
			if (str == "struct")
			{
				result = TokenKind_Struct;
			}
		} break;
	}

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

	TokenKind type = IdentifierType(str);
	Token result = MakeToken(lexer, type, str);
	return result;
}

internal Token
ParseString(Lexer *lexer)
{
	string str = {lexer->current, 0};

	// eat the opening quote
	AdvanceChar(lexer);
	str.count++;

	while (!IsAtEnd(lexer) && PeekChar(lexer) != '"')
	{
		if (PeekChar(lexer) == '\n')
		{
			return ErrorToken(lexer, "newline in string");
		}

		AdvanceChar(lexer);
		str.count++;
	}

	if (IsAtEnd(lexer))
	{
		return ErrorToken(lexer, "unterminated string");
	}

	// eat the closing quote
	AdvanceChar(lexer);
	str.count++;

	TokenKind kind = TokenKind_String;

	if (*lexer->current == 'c')
	{
		kind = TokenKind_CString;

		AdvanceChar(lexer);
		str.count++;
	}

	return MakeToken(lexer, kind, str);
}

internal Token
ParseDecimalNumber(Lexer *lexer)
{
	string str = {lexer->current, 0};
	i64 value = 0;

	while (IsDigit(*lexer->current)
		   || *lexer->current == '_')
	{
		if (*lexer->current == '_')
		{
			// ignore
		}
		else
		{
			value = 10*value + (*lexer->current - '0');
		}

		AdvanceChar(lexer);
		str.count++;
	}

	// look for a fractional part
	if (*lexer->current == '.'
		&& IsDigit(PeekNextChar(lexer)))
	{
		// consume the dot
		AdvanceChar(lexer);
		str.count++;

		while (IsDigit(*lexer->current)
			   || *lexer->current == '_')
		{
			if (*lexer->current == '_')
			{
				// ignore
			}
			else
			{
				// TODO
			}

			AdvanceChar(lexer);
			str.count++;
		}
	}

	Token result = MakeToken(lexer, TokenKind_Number, str);
	result.numberValue = value;
	return result;
}

internal Token
ParseHexadecimalNumber(Lexer *lexer)
{
	string str = {lexer->current, 0};
	i64 value = 0;

	AdvanceChar(lexer, 2); // skip '0x'

	while (IsHexadecimal(*lexer->current)
		   || *lexer->current == '_')
	{
		if (*lexer->current >= '0' && *lexer->current <= '9')
		{
			value = 16*value + (*lexer->current - '0');
		}
		else if (*lexer->current >= 'a' && *lexer->current <= 'f')
		{
			value = 16*value + (*lexer->current - 'a' + 10);
		}
		else if (*lexer->current >= 'A' && *lexer->current <= 'F')
		{
			value = 16*value + (*lexer->current - 'A' + 10);
		}
		else if (*lexer->current == '_')
		{
			// ignore
		}

		AdvanceChar(lexer);
		str.count++;
	}

	Token result = MakeToken(lexer, TokenKind_Number, str);
	result.numberValue = value;
	return result;
}

internal Token
ParseNumber(Lexer *lexer)
{
	if (PeekChar(lexer) == '0'
		&& PeekNextChar(lexer) == 'x')
	{
		return ParseHexadecimalNumber(lexer);
	}

	return ParseDecimalNumber(lexer);
}

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

Token
GetToken(Lexer *lexer)
{
	SkipWhitespace(lexer);

	if (IsAtEnd(lexer))
	{
		return MakeToken(lexer, TokenKind_EOF, {});
	}

	char c = PeekChar(lexer);

	if (IsAlpha(c))
	{
		return ParseIdentifier(lexer);
	}

	if (IsDigit(c))
	{
		return ParseNumber(lexer);
	}

	// handle two-letter tokens
	{
		struct TokenInfo
		{
			string str;
			TokenKind type;
		};

		TokenInfo tokenInfos[] =
		{
			{"!=", TokenKind_BangEqual},
			{"==", TokenKind_EqualEqual},
			{">=", TokenKind_GreaterEqual},
			{"<=", TokenKind_LessEqual},
			{"->", TokenKind_Arrow},
			{"&&", TokenKind_AmpAmp},
			{"||", TokenKind_PipePipe},
			{">>", TokenKind_GreaterGreater},
			{"<<", TokenKind_LessLess},
		};

		for (auto &tokenInfo : tokenInfos)
		{
			if (c == tokenInfo.str[0]
				&& PeekNextChar(lexer) == tokenInfo.str[1])
			{
				string str = {lexer->current, 2};
				TokenKind type = tokenInfo.type;
				AdvanceChar(lexer, 2);

				return MakeToken(lexer, type, str);
			}
		}
	}

	// handle one-letter tokens
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
		|| c == '['
		|| c == ']'
		|| c == '!'
		|| c == '='
		|| c == '>'
		|| c == '<'
		|| c == ':'
		|| c == '&'
		|| c == '%'
		|| c == '#'
		|| c == '|'
		|| c == '^')
	{
		string str = {lexer->current, 1};
		AdvanceChar(lexer);
		return MakeToken(lexer, (TokenKind)c, str);
	}

	if (c == '"')
	{
		return ParseString(lexer);
	}

	return ErrorToken(lexer, "unexpected character");
}
