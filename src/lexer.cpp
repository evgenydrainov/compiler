#include "lexer.h"

#include <stdlib.h>

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
	if (lexer->current[0] != 0)
	{
		result = lexer->current[1];
	}
	return result;
}

internal char
PeekNextNextChar(Lexer *lexer)
{
	char result = 0;
	if (lexer->current[0] != 0
		&& lexer->current[1] != 0)
	{
		result = lexer->current[2];
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

		case 'b':
		{
			if (str == "break")
			{
				result = TokenKind_Break;
			}
		} break;

		case 'c':
		{
			if (str == "cast")
			{
				result = TokenKind_Cast;
			}
			if (str == "continue")
			{
				result = TokenKind_Continue;
			}
		} break;

		case 'd':
		{
			if (str == "do")
			{
				result = TokenKind_Do;
			}
			if (str == "defer")
			{
				result = TokenKind_Defer;
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
			else if (str == "foreach")
			{
				result = TokenKind_Foreach;
			}
		} break;

		case 'i':
		{
			if (str == "if")
			{
				result = TokenKind_If;
			}
			else if (str == "in")
			{
				result = TokenKind_In;
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

		case 'y':
		{
			if (str == "yield")
			{
				result = TokenKind_Yield;
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

	i64 int64Value = 0;
	f64 float64Value = 0;

	TokenKind kind = TokenKind_Int64Literal;

	while (IsDigit(*lexer->current)
		   || *lexer->current == '_')
	{
		if (*lexer->current == '_')
		{
			// ignore
		}
		else
		{
			int64Value = 10*int64Value + (*lexer->current - '0');
		}

		AdvanceChar(lexer);
		str.count++;
	}

	if (*lexer->current == '.'
		&& IsDigit(PeekNextChar(lexer)))
	{
		AdvanceChar(lexer); // eat the '.'
		str.count++;

		kind = TokenKind_Float32Literal; // f32 is the default
		//float64Value = (f64)int64Value;
		//f64 multiplier = 0.1;

		while (IsDigit(*lexer->current)
			   || *lexer->current == '_')
		{
			if (*lexer->current == '_')
			{
				// ignore
			}
			else
			{
				//float64Value += multiplier * (*lexer->current - '0');
				//multiplier /= 10.0;
			}

			AdvanceChar(lexer);
			str.count++;
		}

		// hack
		char saveChar = *lexer->current;
		*lexer->current = 0;
		float64Value = strtod(str.data, nullptr);
		*lexer->current = saveChar;
		// hack

		if (lexer->current[0] == 'f'
			&& lexer->current[1] == '6'
			&& lexer->current[2] == '4')
		{
			kind = TokenKind_Float64Literal;
			AdvanceChar(lexer, 3); // eat the 'f64' postfix
		}

		if (lexer->current[0] == 'f'
			&& lexer->current[1] == '3'
			&& lexer->current[2] == '2')
		{
			kind = TokenKind_Float32Literal;
			AdvanceChar(lexer, 3); // eat the 'f32' postfix
		}
	}

	Token result = MakeToken(lexer, kind, str, location);

	if (kind == TokenKind_Int64Literal)
	{
		result.int64Value = int64Value;
	}
	else if (kind == TokenKind_Float32Literal)
	{
		result.float32Value = (f32)float64Value;
	}
	else if (kind == TokenKind_Float64Literal)
	{
		result.float64Value = float64Value;
	}
	else
	{
		Assert(false);
	}

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

	Token result = MakeToken(lexer, TokenKind_Int64Literal, str, location);
	result.int64Value = value;
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

internal Token
IncludeFile(Lexer *lexer,
			SourceLocation location,
			string filePath)
{
	if (lexer->includeStack.count >= MAX_INCLUDE_DEPTH)
	{
		return ErrorToken(lexer, "#include nesting too deep (circular include?)", location);
	}

	string fileData = read_entire_file(filePath);
	if (fileData.count == 0)
	{
		return ErrorToken(lexer, "cannot open included file", location);
	}

	LexerFrame frame = {};
	frame.current = lexer->current;
	frame.line = lexer->line;
	frame.fileName = lexer->fileName;
	frame.lineStart = lexer->lineStart;

	array_add(&lexer->includeStack, frame);

	lexer->current = fileData.data;
	lexer->line = 1;
	lexer->fileName = filePath;
	lexer->lineStart = lexer->current;

	return GetToken(lexer);
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

	auto AdvanceAndMakeToken = [](Lexer *lexer, TokenKind kind, SourceLocation location, u32 count)
	{
		string str = {lexer->current, count};
		AdvanceChar(lexer, count);

		return MakeToken(lexer, kind, str, location);
	};

	switch (c)
	{
		case '!':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_BangEqual, location, 2);
			}
		} break;

		case '=':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_EqualEqual, location, 2);
			}
		} break;

		case '>':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_GreaterEqual, location, 2);
			}
			if (PeekNextChar(lexer) == '>')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_GreaterGreater, location, 2);
			}
		} break;

		case '<':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_LessEqual, location, 2);
			}
			if (PeekNextChar(lexer) == '<')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_LessLess, location, 2);
			}
		} break;

		case '-':
		{
			if (PeekNextChar(lexer) == '>')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_Arrow, location, 2);
			}
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_MinusEqual, location, 2);
			}
		} break;

		case '&':
		{
			if (PeekNextChar(lexer) == '&')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_AmpAmp, location, 2);
			}
		} break;

		case '|':
		{
			if (PeekNextChar(lexer) == '|')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_PipePipe, location, 2);
			}
		} break;

		case '+':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_PlusEqual, location, 2);
			}
		} break;

		case '%':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_PercentEqual, location, 2);
			}
		} break;

		case '*':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_StarEqual, location, 2);
			}
		} break;

		case '/':
		{
			if (PeekNextChar(lexer) == '=')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_SlashEqual, location, 2);
			}
		} break;

		case '.':
		{
			if (PeekNextChar(lexer) == '.'
				&& PeekNextNextChar(lexer) == '<')
			{
				return AdvanceAndMakeToken(lexer, TokenKind_DotDotLess, location, 3);
			}
		} break;
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
				return ErrorToken(lexer, "expected \"filepath\" after #include", location);
			}

			Token filePathToken = ParseString(lexer, {});

			Assert(filePathToken.str.count >= 2);
			string filePath = { filePathToken.str.data + 1, filePathToken.str.count - 2};

			string searchDir = strip_filename(lexer->fileName);
			string fullFilePath = string_concat(searchDir, filePath);

			return IncludeFile(lexer, location, filePath);
		}

		if (identifier.str == "import")
		{
			SkipWhitespace(lexer);

			if (PeekChar(lexer) != '"')
			{
				return ErrorToken(lexer, "expected \"filepath\" after #import", location);
			}

			Token filePathToken = ParseString(lexer, {});

			Assert(filePathToken.str.count >= 2);
			string filePath = { filePathToken.str.data + 1, filePathToken.str.count - 2};

			string fullFilePath = string_concat(lexer->context->modulesDir, filePath);

			return IncludeFile(lexer, location, fullFilePath);
		}

		*lexer = saveLexer;
	}

	if (c == '"')
	{
		return ParseString(lexer, location);
	}

	if ((c >= '!' && c <= '/')
		|| (c >= ':' && c <= '@')
		|| (c >= '[' && c <= '`')
		|| (c >= '{' && c <= '~'))
	{
		return AdvanceAndMakeToken(lexer, (TokenKind)c, location, 1);
	}

	return ErrorToken(lexer, "unexpected character", location);
}
