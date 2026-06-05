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
ErrorToken(Lexer *lexer, string message)
{
	Token result = {};
	result.type = TokenType_Error;
	result.str = message;
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

	return MakeToken(lexer, TokenType_String, str);
}

internal Token
ParseNumber(Lexer *lexer)
{
	string str = {lexer->current, 0};

	while (IsDigit(PeekChar(lexer)))
	{
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

	return MakeToken(lexer, TokenType_Number, str);
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
		return MakeToken(lexer, TokenType_EOF, {});
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
			TokenType type;
		};

		TokenInfo tokenInfos[] =
		{
			{"!=", TokenType_BangEqual},
			{"==", TokenType_EqualEqual},
			{">=", TokenType_GreaterEqual},
			{"<=", TokenType_LessEqual},
			{"->", TokenType_Arrow},
		};

		for (TokenInfo tokenInfo : tokenInfos)
		{
			if (c == tokenInfo.str[0]
				&& PeekNextChar(lexer) == tokenInfo.str[1])
			{
				string str = {lexer->current, 2};
				TokenType type = tokenInfo.type;
				AdvanceChar(lexer);
				AdvanceChar(lexer);

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
		|| c == ':')
	{
		string str = {lexer->current, 1};
		AdvanceChar(lexer);
		return MakeToken(lexer, (TokenType)c, str);
	}

	if (c == '"')
	{
		return ParseString(lexer);
	}

	return ErrorToken(lexer, "unexpected character");
}
