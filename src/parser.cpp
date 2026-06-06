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
