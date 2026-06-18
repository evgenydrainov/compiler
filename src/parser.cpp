#include "parser.h"
#include <stdio.h>
#include <stdarg.h>

internal int
LeftBindingPower(TokenKind type)
{
	switch (type)
	{
		case TokenKind_Greater:
		case TokenKind_Less:
		case TokenKind_BangEqual:
		case TokenKind_EqualEqual:
		case TokenKind_GreaterEqual:
		case TokenKind_LessEqual:
			return 1;

		case TokenKind_Plus:
		case TokenKind_Minus:
			return 2;

		case TokenKind_Asterisk:
		case TokenKind_Slash:
			return 3;

		default:
			return 0;
	}
}

internal AstNode *
MakeNode(NodeType type,
		 int line,
		 Arena *arena)
{
	AstNode *node = PushStruct(arena, AstNode);
	*node = {};
	node->type = type;
	node->line = line;

	return node;
}

internal AstNode *
MakeBinaryNode(NodeType type,
			   int line,
			   AstNode *lhs,
			   AstNode *rhs,
			   Arena *arena)
{
	AstNode *node = MakeNode(type, line, arena);
	node->binary.lhs = lhs;
	node->binary.rhs = rhs;

	return node;
}

internal AstNode *
MakeNumberNode(int line,
			   int numberValue,
			   Arena *arena)
{
	AstNode *node = MakeNode(NodeType_Number, line, arena);
	node->number.value = numberValue;

	return node;
}

internal void
ErrorAtTokenVA(Parser *parser,
			   Token token,
			   const char *format,
			   va_list args)
{
	fprintf(stderr, "line %d: ", token.line);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");

	parser->hadError = true;
}

internal void
ErrorAtToken(Parser *parser,
			 Token token,
			 const char *format,
			 ...)
{
	va_list args;
	va_start(args, format);
	ErrorAtTokenVA(parser, token, format, args);
	va_end(args);
}

internal void
ErrorAtCurrent(Parser *parser,
			   const char *format,
			   ...)
{
	va_list args;
	va_start(args, format);
	ErrorAtTokenVA(parser, parser->current, format, args);
	va_end(args);
}

internal void
AdvanceToken(Parser *parser, Lexer *lexer)
{
	parser->current = GetToken(lexer);
}

internal bool
ExpectToken(Parser *parser,
			Lexer *lexer,
			TokenKind kind)
{
	bool result = false;

	if (parser->current.kind == kind)
	{
		AdvanceToken(parser, lexer);
		result = true;
	}
	else
	{
		ErrorAtCurrent(parser,
					   "expected '%s', but got '%s'",
					   GetTokenKindPrettyName(kind),
					   GetTokenKindPrettyName(parser->current.kind));
	}

	return result;
}

internal void
UnexpectedCurrentToken(Parser *parser)
{
	ErrorAtCurrent(parser, "unexpected token '%s'", GetTokenKindPrettyName(parser->current.kind));
}

internal AstNode *
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower,
				Arena *arena);

internal AstNode *
ParseAtom(Parser *parser,
		  Lexer *lexer,
		  Arena *arena)
{
	AstNode *node = nullptr;

	switch (parser->current.kind)
	{
		case TokenKind_Number:
		{
			node = MakeNumberNode(parser->current.line, parser->current.numberValue, arena);
			AdvanceToken(parser, lexer);
		} break;

		case TokenKind_OpenParen:
		{
			AdvanceToken(parser, lexer);
			node = ParseExpression(parser, lexer, 0, arena);
			ExpectToken(parser, lexer, TokenKind_CloseParen);
		} break;

		case TokenKind_Minus:
		{
			// unary minus

			node = MakeNode(NodeType_Subtract, parser->current.line, arena);
			node->binary.lhs = MakeNumberNode(parser->current.line, 0, arena);
			AdvanceToken(parser, lexer);
			node->binary.rhs = ParseAtom(parser, lexer, arena);
		} break;

		case TokenKind_Identifier:
		{
			if (PeekToken(lexer).kind == TokenKind_OpenParen)
			{
				// function call
				// foo(a, b, c)

				const int MAX_ARGUMENTS = 32;

				node = MakeNode(NodeType_Call, parser->current.line, arena);
				node->call.name = parser->current.str;
				node->call.expressions = PushArray(arena, MAX_ARGUMENTS, AstNode *);
				node->call.numExpressions = 0;

				AdvanceToken(parser, lexer); // eat the function name
				AdvanceToken(parser, lexer); // eat the '('

				while (!parser->hadError
					   && parser->current.kind != TokenKind_CloseParen)
				{
					if (node->call.numExpressions != 0)
					{
						ExpectToken(parser, lexer, TokenKind_Comma);
					}

					Assert(node->call.numExpressions < MAX_ARGUMENTS);
					node->call.expressions[node->call.numExpressions] = ParseExpression(parser, lexer, 0, arena);

					node->call.numExpressions++;
				}

				ExpectToken(parser, lexer, TokenKind_CloseParen);
			}
			else
			{
				// variable access

				node = MakeNode(NodeType_Var, parser->current.line, arena);
				node->var.name = parser->current.str;

				AdvanceToken(parser, lexer);
			}
		} break;

		case TokenKind_True:
		{
			node = MakeNode(NodeType_Bool, parser->current.line, arena);
			node->_bool.value = true;
			AdvanceToken(parser, lexer);
		} break;

		case TokenKind_False:
		{
			node = MakeNode(NodeType_Bool, parser->current.line, arena);
			node->_bool.value = false;
			AdvanceToken(parser, lexer);
		} break;

		case TokenKind_Ampersand:
		{
			node = MakeNode(NodeType_AddressOf, parser->current.line, arena);
			AdvanceToken(parser, lexer);

			node->addressOf.what = ParseAtom(parser, lexer, arena);
		} break;

		case TokenKind_Asterisk:
		{
			node = MakeNode(NodeType_Deref, parser->current.line, arena);
			AdvanceToken(parser, lexer);

			node->deref.what = ParseAtom(parser, lexer, arena);
		} break;

		default:
		{
			UnexpectedCurrentToken(parser);
		} break;
	}
	
	return node;
}

internal AstNode *
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower,
				Arena *arena)
{
	AstNode *lhs = ParseAtom(parser, lexer, arena);
	for (;;)
	{
		int bindingPower = LeftBindingPower(parser->current.kind);
		if (bindingPower <= minBindingPower)
		{
			break;
		}

		TokenKind type = parser->current.kind;
		int line = parser->current.line;

		AdvanceToken(parser, lexer);

		AstNode *rhs = ParseExpression(parser, lexer, bindingPower, arena);
		switch (type)
		{
			case TokenKind_Plus:
			{
				lhs = MakeBinaryNode(NodeType_Add, line, lhs, rhs, arena);
			} break;

			case TokenKind_Minus:
			{
				lhs = MakeBinaryNode(NodeType_Subtract, line, lhs, rhs, arena);
			} break;

			case TokenKind_Asterisk:
			{
				lhs = MakeBinaryNode(NodeType_Multiply, line, lhs, rhs, arena);
			} break;

			case TokenKind_Slash:
			{
				lhs = MakeBinaryNode(NodeType_Divide, line, lhs, rhs, arena);
			} break;

			case TokenKind_Less:
			{
				lhs = MakeBinaryNode(NodeType_Less, line, lhs, rhs, arena);
			} break;

			case TokenKind_Greater:
			{
				lhs = MakeBinaryNode(NodeType_Greater, line, lhs, rhs, arena);
			} break;

			case TokenKind_EqualEqual:
			{
				lhs = MakeBinaryNode(NodeType_EqualEqual, line, lhs, rhs, arena);
			} break;

			case TokenKind_LessEqual:
			{
				lhs = MakeBinaryNode(NodeType_LessEqual, line, lhs, rhs, arena);
			} break;

			case TokenKind_GreaterEqual:
			{
				lhs = MakeBinaryNode(NodeType_GreaterEqual, line, lhs, rhs, arena);
			} break;

			case TokenKind_BangEqual:
			{
				lhs = MakeBinaryNode(NodeType_NotEqual, line, lhs, rhs, arena);
			} break;

			default: {} break;
		}
	}

	return lhs;
}

internal AstNode *
ParseStatement(Parser *parser,
			   Lexer *lexer,
			   Arena *arena);

internal AstNode *
ParseBlock(Parser *parser,
		   Lexer *lexer,
		   Arena *arena)
{
	const int MAX_STATEMENTS = 1024;

	int openBraceTokenLine = parser->current.line;

	if (!ExpectToken(parser, lexer, TokenKind_OpenBrace))
	{
		return nullptr;
	}

	AstNode *block = MakeNode(NodeType_Block, openBraceTokenLine, arena);
	block->block.statements = PushArray(arena, MAX_STATEMENTS, AstNode *);
	block->block.numStatements = 0;

	while (parser->current.kind != TokenKind_CloseBrace
		   && !parser->hadError)
	{
		AstNode *statement = ParseStatement(parser, lexer, arena);
		if (parser->hadError)
		{
			break;
		}

		Assert(block->block.numStatements < MAX_STATEMENTS);
		block->block.statements[block->block.numStatements++] = statement;
	}

	if (!ExpectToken(parser, lexer, TokenKind_CloseBrace))
	{
		return nullptr;
	}

	return block;
}

internal Type
ParseType(Parser *parser,
		  Lexer *lexer,
		  Arena *arena)
{
	Type type = {};

	if (parser->current.kind == TokenKind_Asterisk)
	{
		type.kind = TypeKind_Pointer;
		AdvanceToken(parser, lexer); // eat the '*'

		Type pointerTo = ParseType(parser, lexer, arena);

		type.pointerTo = PushStruct(arena, Type);
		*type.pointerTo = pointerTo;
	}
	else if (parser->current.str == "i64")
	{
		type.kind = TypeKind_Int64;
		AdvanceToken(parser, lexer);
	}
	else if (parser->current.str == "bool")
	{
		type.kind = TypeKind_Bool;
		AdvanceToken(parser, lexer);
	}
	else
	{
		ErrorAtCurrent(parser, STR_FMT_QUOTED ": unknown type", STR_ARG(parser->current.str));
	}

	return type;
}

internal AstNode *
ParseReturnStatement(Parser *parser,
					 Lexer *lexer,
					 Arena *arena)
{
	AstNode *node = MakeNode(NodeType_Return, parser->current.line, arena);

	// eat the 'return'
	AdvanceToken(parser, lexer);

	if (parser->current.kind == TokenKind_Semicolon)
	{
		// bare return;
	}
	else
	{
		// return expr;

		node->ret.expr = ParseExpression(parser, lexer, 0, arena);	
	}

	ExpectToken(parser, lexer, TokenKind_Semicolon);

	return node;
}

internal AstNode *
ParseIfStatement(Parser *parser,
				 Lexer *lexer,
				 Arena *arena)
{
	// if expr { statements; }

	AstNode *node = MakeNode(NodeType_If, parser->current.line, arena);

	// eat the 'if'
	AdvanceToken(parser, lexer);

	node->_if.condition = ParseExpression(parser, lexer, 0, arena);	

	node->_if.thenBlock = ParseBlock(parser, lexer, arena);

	if (parser->current.kind == TokenKind_Else)
	{
		// eat the 'else'
		AdvanceToken(parser, lexer);

		node->_if.elseBlock = ParseBlock(parser, lexer, arena);
	}

	return node;
}

internal AstNode *
ParseWhileStatement(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	// while expr { statements; }

	AstNode *node = MakeNode(NodeType_While, parser->current.line, arena);

	// eat the 'while'
	AdvanceToken(parser, lexer);

	node->_while.condition = ParseExpression(parser, lexer, 0, arena);	

	node->_while.body = ParseBlock(parser, lexer, arena);

	return node;
}

internal AstNode *
ParsePrintStatement(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	// print expr;

	AstNode *node = MakeNode(NodeType_Print, parser->current.line, arena);

	// eat the 'print'
	AdvanceToken(parser, lexer);

	node->print.expr = ParseExpression(parser, lexer, 0, arena);	

	ExpectToken(parser, lexer, TokenKind_Semicolon);

	return node;
}

internal AstNode *
ParseVariableDeclaration(Parser *parser,
						 Lexer *lexer,
						 Arena *arena)
{
	// variable declaration

	AstNode *node = MakeNode(NodeType_VarDecl, parser->current.line, arena);
	node->varDecl.name = parser->current.str;

	// eat the variable name
	AdvanceToken(parser, lexer);

	ExpectToken(parser, lexer, TokenKind_Colon);

	node->varDecl.type = ParseType(parser, lexer, arena);

	if (parser->current.kind == TokenKind_Equal)
	{
		// name : type = expr;

		// eat the '='
		AdvanceToken(parser, lexer);

		node->varDecl.expr = ParseExpression(parser, lexer, 0, arena);

		ExpectToken(parser, lexer, TokenKind_Semicolon);
	}
	else if (parser->current.kind == TokenKind_Semicolon)
	{
		// name : type;

		// eat the semicolon
		AdvanceToken(parser, lexer);

		// initialize to zero by default
		// node->varDecl.expr = MakeNumberNode(parser->current.line, 0, arena);
	}
	else
	{
		ErrorAtCurrent(parser, "expected '=' or ';'");
	}

	return node;
}

internal AstNode *
ParseAssignmentStatement(Parser *parser,
						 Lexer *lexer,
						 Arena *arena)
{
	// assignment
	// name = expr;

	AstNode *node = MakeNode(NodeType_Assign, parser->current.line, arena);
	node->assign.name = parser->current.str;

	// eat the variable name
	AdvanceToken(parser, lexer);

	ExpectToken(parser, lexer, TokenKind_Equal);

	node->assign.expr = ParseExpression(parser, lexer, 0, arena);

	ExpectToken(parser, lexer, TokenKind_Semicolon);

	return node;
}

internal AstNode *
ParseStatement(Parser *parser,
			   Lexer *lexer,
			   Arena *arena)
{
	AstNode *node = nullptr;

	switch (parser->current.kind)
	{
		case TokenKind_If:
		{
			node = ParseIfStatement(parser, lexer, arena);
		} break;

		case TokenKind_While:
		{
			node = ParseWhileStatement(parser, lexer, arena);
		} break;

		case TokenKind_Print:
		{
			node = ParsePrintStatement(parser, lexer, arena);
		} break;

		case TokenKind_OpenBrace:
		{
			// empty scope
			// { statements; }
			node = ParseBlock(parser, lexer, arena);
		} break;

		case TokenKind_Return:
		{
			node = ParseReturnStatement(parser, lexer, arena);
		} break;

		case TokenKind_Identifier:
		{
			Token peekToken = PeekToken(lexer);

			if (peekToken.kind == TokenKind_Colon)
			{
				node = ParseVariableDeclaration(parser, lexer, arena);
			}
			else if (peekToken.kind == TokenKind_Equal)
			{
				node = ParseAssignmentStatement(parser, lexer, arena);
			}
			else
			{
				// bare expression
				// expr;
				node = ParseExpression(parser, lexer, 0, arena);
				ExpectToken(parser, lexer, TokenKind_Semicolon);
			}
		} break;

		default:
		{
			// bare expression
			// expr;
			node = ParseExpression(parser, lexer, 0, arena);
			ExpectToken(parser, lexer, TokenKind_Semicolon);
		} break;
	}

	return node;
}

internal AstNode *
ParseFunctionDefinition(Parser *parser,
						Lexer *lexer,
						Arena *arena)
{
	// function definition
	// main :: proc(foo: int) -> i64 { statements; }

	const int MAX_ARGUMENTS = 32;

	AstNode *node = MakeNode(NodeType_Func, parser->current.line, arena);
	node->func.name = parser->current.str;
	node->func.params = PushArray(arena, MAX_ARGUMENTS, AstNode *);
	node->func.numParams = 0;

	// eat the function name
	AdvanceToken(parser, lexer);

	ExpectToken(parser, lexer, TokenKind_Colon);
	ExpectToken(parser, lexer, TokenKind_Colon);

	ExpectToken(parser, lexer, TokenKind_Proc);

	ExpectToken(parser, lexer, TokenKind_OpenParen);

	while (!parser->hadError
			&& parser->current.kind != TokenKind_CloseParen)
	{
		if (node->func.numParams != 0)
		{
			ExpectToken(parser, lexer, TokenKind_Comma);
		}

		AstNode *param = MakeNode(NodeType_Param, parser->current.line, arena);
		param->param.name = parser->current.str;

		ExpectToken(parser, lexer, TokenKind_Identifier); // eat the param name

		ExpectToken(parser, lexer, TokenKind_Colon);

		param->param.type = ParseType(parser, lexer, arena);

		Assert(node->func.numParams < MAX_ARGUMENTS);
		node->func.params[node->func.numParams] = param;

		node->func.numParams++;
	}

	AdvanceToken(parser, lexer); // eat the ')'

	node->func.returnType.kind = TypeKind_Void;

	if (parser->current.kind == TokenKind_Arrow)
	{
		AdvanceToken(parser, lexer); // eat the '->'

		node->func.returnType = ParseType(parser, lexer, arena);
	}

	node->func.body = ParseBlock(parser, lexer, arena);

	return node;
}

internal AstNode *
ParseTopLevelStatement(Parser *parser,
					   Lexer *lexer,
					   Arena *arena)
{
	AstNode *node = nullptr;

	switch (parser->current.kind)
	{
		case TokenKind_Identifier:
		{
			node = ParseFunctionDefinition(parser, lexer, arena);
		} break;

		default:
		{
			UnexpectedCurrentToken(parser);
		} break;
	}

	return node;
}

AstNode *
ParseProgram(Parser *parser,
			 Lexer *lexer,
			 Arena *arena)
{
	const int MAX_STATEMENTS = 1024;

	AstNode *block = MakeNode(NodeType_Block, 1, arena);
	block->block.statements = PushArray(arena, MAX_STATEMENTS, AstNode *);
	block->block.numStatements = 0;

	while (!parser->hadError
		   && parser->current.kind != TokenKind_EOF)
	{
		AstNode *statement = ParseTopLevelStatement(parser, lexer, arena);
		if (parser->hadError)
		{
			break;
		}

		Assert(block->block.numStatements < MAX_STATEMENTS);
		block->block.statements[block->block.numStatements++] = statement;
	}

	return block;
}
