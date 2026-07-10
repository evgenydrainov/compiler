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
MakeToken(Lexer *lexer, TokenKind kind, string str, SourceLocation location)
{
	Token result = {};
	result.kind = kind;
	result.str = str;
	result.location = location;

	return result;
}

internal Token
ErrorToken(Lexer *lexer, string message, SourceLocation location)
{
	Token result = {};
	result.kind = TokenKind_Error;
	result.str = message;
	result.location = location;

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

		case 'c':
		{
			if (str == "cast")
			{
				result = TokenKind_Cast;
			}
		} break;

		case 'd':
		{
			if (str == "do")
			{
				result = TokenKind_Do;
			}
		} break;

		case 'e':
		{
			if (str == "else")
			{
				result = TokenKind_Else;
			}
			if (str == "enum")
			{
				result = TokenKind_Enum;
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

		case 'i':
		{
			if (str == "if")
			{
				result = TokenKind_If;
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

		case 's':
		{
			if (str == "struct")
			{
				result = TokenKind_Struct;
			}
		} break;

		case 't':
		{
			if (str == "true")
			{
				result = TokenKind_True;
			}
		} break;

		case 'w':
		{
			if (str == "while")
			{
				result = TokenKind_While;
			}
		} break;
	}

	return result;
}

internal Token
ParseIdentifier(Lexer *lexer, SourceLocation location)
{
	string str = {lexer->current, 0};

	while (IsAlpha(PeekChar(lexer))
		   || IsDigit(PeekChar(lexer)))
	{
		AdvanceChar(lexer);
		str.count++;
	}

	TokenKind type = IdentifierType(str);
	Token result = MakeToken(lexer, type, str, location);
	return result;
}

internal Token
ParseString(Lexer *lexer, SourceLocation location)
{
	string str = {lexer->current, 0};

	// eat the opening quote
	AdvanceChar(lexer);
	str.count++;

	while (!IsAtEnd(lexer)
		   && *lexer->current != '"')
	{
		if (*lexer->current == '\n')
		{
			return ErrorToken(lexer, "newline in string", location);
		}

		if (*lexer->current == '\\')
		{
			AdvanceChar(lexer);
			str.count++;

			if (*lexer->current == 'n'
				|| *lexer->current == 't'
				|| *lexer->current == 'r'
				|| *lexer->current == '0'
				|| *lexer->current == '\\'
				|| *lexer->current == '"')
			{
				AdvanceChar(lexer);
				str.count++;
			}
			else
			{
				return ErrorToken(lexer, "invalid escape sequence", location);
			}
		}
		else
		{
			AdvanceChar(lexer);
			str.count++;
		}
	}

	if (IsAtEnd(lexer))
	{
		return ErrorToken(lexer, "unterminated string", location);
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

	return MakeToken(lexer, kind, str, location);
}

internal Token
ParseDecimalNumber(Lexer *lexer, SourceLocation location)
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

	Token result = MakeToken(lexer, TokenKind_Number, str, location);
	result.numberValue = value;
	return result;
}

internal Token
ParseHexadecimalNumber(Lexer *lexer, SourceLocation location)
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

	Token result = MakeToken(lexer, TokenKind_Number, str, location);
	result.numberValue = value;
	return result;
}

internal Token
ParseNumber(Lexer *lexer, SourceLocation location)
{
	if (PeekChar(lexer) == '0'
		&& PeekNextChar(lexer) == 'x')
	{
		return ParseHexadecimalNumber(lexer, location);
	}

	return ParseDecimalNumber(lexer, location);
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
			lexer->lineStart = lexer->current;
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

	SourceLocation location =
	{
		.fileName = lexer->fileName,
		.line     = lexer->line,
		.column   = (int)(lexer->current - lexer->lineStart + 1),
	};

	if (IsAtEnd(lexer))
	{
		if (lexer->includeStack.count > 0)
		{
			LexerFrame frame = lexer->includeStack[lexer->includeStack.count - 1];
			lexer->includeStack.count--;

			lexer->current = frame.current;
			lexer->line = frame.line;
			lexer->fileName = frame.fileName;
			lexer->lineStart = frame.lineStart;

			return GetToken(lexer);
		}

		return MakeToken(lexer, TokenKind_EOF, {}, location);
	}

	char c = PeekChar(lexer);

	if (IsAlpha(c))
	{
		return ParseIdentifier(lexer, location);
	}

	if (IsDigit(c))
	{
		return ParseNumber(lexer, location);
	}

	// handle two-character tokens
	{
		struct TwoCharacterTokenInfo
		{
			char str[3];
			TokenKind type;
		};

		TwoCharacterTokenInfo tokenInfos[] =
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
			{"+=", TokenKind_PlusEqual},
			{"-=", TokenKind_MinusEqual},
			{"%=", TokenKind_PercentEqual},
		};

		for (auto &tokenInfo : tokenInfos)
		{
			if (c == tokenInfo.str[0]
				&& PeekNextChar(lexer) == tokenInfo.str[1])
			{
				string str = {lexer->current, 2};
				TokenKind type = tokenInfo.type;
				AdvanceChar(lexer, 2);

				return MakeToken(lexer, type, str, location);
			}
		}
	}

	if (c == '#'
		|| PeekNextChar(lexer) == 'i')
	{
		Lexer saveLexer = *lexer;

		AdvanceChar(lexer); // eat the '#'
		Token identifier = ParseIdentifier(lexer, {});

		if (identifier.str == "include")
		{
			SkipWhitespace(lexer);

			if (PeekChar(lexer) != '"')
			{
				return ErrorToken(lexer, "expected \"filename\" after #include", location);
			}

			Token pathToken = ParseString(lexer, {});

			Assert(pathToken.str.count >= 2);
			string path = { pathToken.str.data + 1, pathToken.str.count - 2};

			if (lexer->includeStack.count >= MAX_INCLUDE_DEPTH)
			{
				return ErrorToken(lexer, "#include nesting too deep (circular include?)", location);
			}

			string fileData = LoadFile(path);
			if (fileData.count == 0)
			{
				return ErrorToken(lexer, "cannot open included file", location);
			}

			LexerFrame frame = {};
			frame.current = lexer->current;
			frame.line = lexer->line;
			frame.fileName = lexer->fileName;
			frame.lineStart = lexer->lineStart;

			ArrayAdd(&lexer->includeStack, frame);

			lexer->current = fileData.data;
			lexer->line = 1;
			lexer->fileName = path;
			lexer->lineStart = lexer->current;

			return GetToken(lexer);
		}

		*lexer = saveLexer;
	}

	// handle single-character tokens
	if (c == '!'
		|| c == '#'
		|| c == '%'
		|| c == '&'
		|| c == '('
		|| c == ')'
		|| c == '*'
		|| c == '+'
		|| c == ','
		|| c == '-'
		|| c == '.'
		|| c == '/'
		|| c == ':'
		|| c == ';'
		|| c == '<'
		|| c == '='
		|| c == '>'
		|| c == '['
		|| c == ']'
		|| c == '^'
		|| c == '{'
		|| c == '|'
		|| c == '}'
		|| c == '~')
	{
		string str = {lexer->current, 1};
		AdvanceChar(lexer);
		return MakeToken(lexer, (TokenKind)c, str, location);
	}

	if (c == '"')
	{
		return ParseString(lexer, location);
	}

	return ErrorToken(lexer, "unexpected character", location);
}
