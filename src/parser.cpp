#include "parser.h"
#include <stdio.h>

internal int
LeftBindingPower(TokenType type)
{
	switch (type)
	{
		case TokenType_Plus:     return 1;
		case TokenType_Minus:    return 1;
		case TokenType_Asterisk: return 2;
		case TokenType_Slash:    return 2;

		default: return 0;
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
	  const char *message)
{
	fprintf(stderr, "%s\n", message);
	
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

	Error(parser, "expected number or '('");

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

			default: {} break;
		}
	}

	return lhs;
}

internal AstNode *
ParseStatement(Parser *parser, Lexer *lexer, Arena *arena)
{
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

		if (parser->current.type != TokenType_Identifier)
		{
			Error(parser, "expected type after ':'");
			return nullptr;
		}

		// eat the type
		NextToken(parser, lexer);
		
		if (parser->current.type == TokenType_Equal)
		{
			// name : type = expr;

			// eat the '='
			NextToken(parser, lexer);

			AstNode *rhs = ParseExpression(parser, lexer, 0, arena);

			if (parser->current.type != TokenType_Semicolon)
			{
				Error(parser, "expected ';'");
				return nullptr;
			}

			// eat the semicolon
			NextToken(parser, lexer);

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

		if (parser->current.type != TokenType_Semicolon)
		{
			Error(parser, "expected ';'");
			return nullptr;
		}

		// eat the semicolon
		NextToken(parser, lexer);

		return node;
	}

	return nullptr;
}

AstNode *
ParseProgram(Parser *parser,
			 Lexer *lexer,
			 Arena *arena)
{
	AstNode *block = MakeNode(NodeType_Block, nullptr, nullptr, arena);
	block->statements = PushArray(arena, 1024, AstNode *);
	block->numStatements = 0;

	while (parser->current.type != TokenType_EOF
		   && !parser->hadError)
	{
		AstNode *statement = ParseStatement(parser, lexer, arena);
		if (parser->hadError)
		{
			break;
		}

		Assert(block->numStatements < 1024);
		block->statements[block->numStatements++] = statement;
	}

	return block;
}
