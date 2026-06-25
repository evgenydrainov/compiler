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

	return MakeToken(lexer, TokenKind_String, str);
}

internal Token
ParseNumber(Lexer *lexer)
{
	string str = {lexer->current, 0};
	int value = 0;

	while (IsDigit(PeekChar(lexer)))
	{
		value = 10*value + (PeekChar(lexer) - '0');

		AdvanceChar(lexer);
		str.count++;
	}

	// look for a fractional part
	if (PeekChar(lexer) == '.'
		&& IsDigit(PeekNextChar(lexer)))
	{
		// consume the dot
		AdvanceChar(lexer);
		str.count++;

		while (IsDigit(PeekChar(lexer)))
		{
			AdvanceChar(lexer);
			str.count++;
		}
	}

	Token result = MakeToken(lexer, TokenKind_Number, str);
	result.numberValue = value;
	return result;
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
		|| c == '%')
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
