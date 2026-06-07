#include "parser.h"
#include <stdio.h>
#include <stdarg.h>

internal int
LeftBindingPower(TokenType type)
{
	switch (type)
	{
		case TokenType_Greater:
		case TokenType_Less:
		case TokenType_BangEqual:
		case TokenType_EqualEqual:
		case TokenType_GreaterEqual:
		case TokenType_LessEqual:
			return 1;

		case TokenType_Plus:
		case TokenType_Minus:
			return 2;

		case TokenType_Asterisk:
		case TokenType_Slash:
			return 3;

		default:
			return 0;
	}
}

internal AstNode *
MakeNode(NodeType type,
		 AstNode *lhs,
		 AstNode *rhs,
		 Arena *arena)
{
	AstNode *node = PushStruct(arena, AstNode);
	*node = {};
	node->type = type;
	node->lhs = lhs;
	node->rhs = rhs;

	return node;
}

internal AstNode *
MakeNumberNode(int numberValue,
			   Arena *arena)
{
	AstNode *node = PushStruct(arena, AstNode);
	*node = {};
	node->type = NodeType_Number;
	node->numberValue = numberValue;

	return node;
}

internal void
Error(Parser *parser,
	  const char *format,
	  ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");

	va_end(args);
	
	parser->hadError = true;
}

internal void
NextToken(Parser *parser, Lexer *lexer)
{
	parser->current = GetToken(lexer);
}

internal AstNode *
ParseAtom(Parser *parser,
		  Lexer *lexer,
		  Arena *arena)
{
	if (parser->current.type == TokenType_Number)
	{
		AstNode *node = MakeNumberNode(parser->current.numberValue, arena);
		NextToken(parser, lexer);

		return node;
	}

	if (parser->current.type == TokenType_LeftParen)
	{
		NextToken(parser, lexer);

		AstNode *node = ParseExpression(parser, lexer, 0, arena);

		if (parser->current.type != TokenType_RightParen)
		{
			Error(parser, "expected ')'");
		}

		NextToken(parser, lexer);

		return node;
	}

	if (parser->current.type == TokenType_Minus)
	{
		NextToken(parser, lexer);

		AstNode *lhs = MakeNumberNode(0, arena);
		AstNode *rhs = ParseAtom(parser, lexer, arena);

		return MakeNode(NodeType_Subtract, lhs, rhs, arena);
	}

	if (parser->current.type == TokenType_Identifier)
	{
		AstNode *node = MakeNode(NodeType_Var, nullptr, nullptr, arena);
		node->name = parser->current.str;
		NextToken(parser, lexer);

		return node;
	}

	Error(parser, "unexpected token %s", GetTokenTypeName(parser->current.type));

	return nullptr;
}

AstNode *
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower,
				Arena *arena)
{
	AstNode *lhs = ParseAtom(parser, lexer, arena);
	for (;;)
	{
		int bindingPower = LeftBindingPower(parser->current.type);
		if (bindingPower <= minBindingPower)
		{
			break;
		}

		TokenType type = parser->current.type;
		NextToken(parser, lexer);

		AstNode *rhs = ParseExpression(parser, lexer, bindingPower, arena);
		switch (type)
		{
			case TokenType_Plus:
			{
				lhs = MakeNode(NodeType_Add, lhs, rhs, arena);
			} break;

			case TokenType_Minus:
			{
				lhs = MakeNode(NodeType_Subtract, lhs, rhs, arena);
			} break;

			case TokenType_Asterisk:
			{
				lhs = MakeNode(NodeType_Multiply, lhs, rhs, arena);
			} break;

			case TokenType_Slash:
			{
				lhs = MakeNode(NodeType_Divide, lhs, rhs, arena);
			} break;

			case TokenType_Less:
			{
				lhs = MakeNode(NodeType_Less, lhs, rhs, arena);
			} break;

			case TokenType_Greater:
			{
				lhs = MakeNode(NodeType_Greater, lhs, rhs, arena);
			} break;

			case TokenType_Equal:
			{
				lhs = MakeNode(NodeType_Equal, lhs, rhs, arena);
			} break;

			default: {} break;
		}
	}

	return lhs;
}

internal bool
ExpectAndEatToken(Parser *parser,
				  Lexer *lexer,
				  TokenType type)
{
	bool result = false;

	if (parser->current.type == type)
	{
		NextToken(parser, lexer);
		result = true;
	}
	else
	{
		Error(parser,
			  "expected %s, but got %s",
			  GetTokenTypeName(type),
			  GetTokenTypeName(parser->current.type));
	}

	return result;
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

	if (!ExpectAndEatToken(parser, lexer, TokenType_LeftBrace))
	{
		return nullptr;
	}

	AstNode *block = MakeNode(NodeType_Block, nullptr, nullptr, arena);
	block->statements = PushArray(arena, MAX_STATEMENTS, AstNode *);
	block->numStatements = 0;

	while (parser->current.type != TokenType_RightBrace
		   && !parser->hadError)
	{
		AstNode *statement = ParseStatement(parser, lexer, arena);
		if (parser->hadError)
		{
			break;
		}

		Assert(block->numStatements < MAX_STATEMENTS);
		block->statements[block->numStatements++] = statement;
	}

	if (!ExpectAndEatToken(parser, lexer, TokenType_RightBrace))
	{
		return nullptr;
	}

	return block;
}

internal AstNode *
ParseStatement(Parser *parser,
			   Lexer *lexer,
			   Arena *arena)
{
	// if expr { statements; }
	if (parser->current.type == TokenType_If)
	{
		// eat the 'if'
		NextToken(parser, lexer);

		AstNode *condition = ParseExpression(parser, lexer, 0, arena);	

		if (parser->hadError)
		{
			return nullptr;
		}

		AstNode *thenBlock = ParseBlock(parser, lexer, arena);
		AstNode *elseBlock = nullptr;

		if (parser->current.type == TokenType_Else)
		{
			// eat the 'else'
			NextToken(parser, lexer);

			elseBlock = ParseBlock(parser, lexer, arena);
		}

		AstNode *node = MakeNode(NodeType_If, nullptr, nullptr, arena);
		node->condition = condition;
		node->thenBlock = thenBlock;
		node->elseBlock = elseBlock;

		return node;
	}

	// while expr { statements; }
	if (parser->current.type == TokenType_While)
	{
		// eat the 'while'
		NextToken(parser, lexer);

		AstNode *condition = ParseExpression(parser, lexer, 0, arena);	

		if (parser->hadError)
		{
			return nullptr;
		}

		AstNode *loopBody = ParseBlock(parser, lexer, arena);

		AstNode *node = MakeNode(NodeType_While, nullptr, nullptr, arena);
		node->condition = condition;
		node->thenBlock = loopBody;

		return node;
	}

	AstNode *expr = ParseExpression(parser, lexer, 0, arena);

	if (parser->hadError)
	{
		return nullptr;
	}

	// variable declaration
	// name : type = expr;
	// name : type;
	if (parser->current.type == TokenType_Colon)
	{
		if (expr->type != NodeType_Var)
		{
			Error(parser, "expected name before ':'");
			return nullptr;
		}

		string name = expr->name;

		// eat the colon
		NextToken(parser, lexer);

		if (!ExpectAndEatToken(parser, lexer, TokenType_Identifier))
		{
			return nullptr;
		}
		
		if (parser->current.type == TokenType_Equal)
		{
			// name : type = expr;

			// eat the '='
			NextToken(parser, lexer);

			AstNode *rhs = ParseExpression(parser, lexer, 0, arena);

			if (!ExpectAndEatToken(parser, lexer, TokenType_Semicolon))
			{
				return nullptr;
			}

			AstNode *node = MakeNode(NodeType_VarDecl, nullptr, rhs, arena);
			node->name = name;
			return node;
		}
		else if (parser->current.type == TokenType_Semicolon)
		{
			// name : type;

			// eat the semicolon
			NextToken(parser, lexer);

			// initialize to zero by default
			AstNode *rhs = MakeNumberNode(0, arena);

			AstNode *node = MakeNode(NodeType_VarDecl, nullptr, rhs, arena);
			node->name = name;
			return node;
		}

		Error(parser, "expected '=' or ';' in declaration");
		return nullptr;
	}

	// assignment
	// name = expr;
	if (parser->current.type == TokenType_Equal)
	{
		if (expr->type != NodeType_Var)
		{
			Error(parser, "invalid assignment target");
			return nullptr;
		}

		string name = expr->name;

		NextToken(parser, lexer);
		AstNode *rhs = ParseExpression(parser, lexer, 0, arena);

		AstNode *node = MakeNode(NodeType_Assign, nullptr, rhs, arena);
		node->name = name;

		if (!ExpectAndEatToken(parser, lexer, TokenType_Semicolon))
		{
			return nullptr;
		}

		return node;
	}

	// bare expression
	// expr;
	if (!ExpectAndEatToken(parser, lexer, TokenType_Semicolon))
	{
		return nullptr;
	}

	return expr;
}

AstNode *
ParseProgram(Parser *parser,
			 Lexer *lexer,
			 Arena *arena)
{
	AstNode *block = ParseBlock(parser, lexer, arena);

	return block;
}
