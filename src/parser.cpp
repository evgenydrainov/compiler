#include "parser.h"

internal int
LeftBindingPower(TokenType type)
{
	return 0;
}

internal AstNode *
ParseAtom(Parser *parser)
{
	return 0;
}

AstNode *
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower)
{
	AstNode *lhs = ParseAtom(parser);
	for (;;)
	{
		int bindingPower = LeftBindingPower(parser->current.type);
		if (bindingPower <= minBindingPower)
		{
			break;
		}

		TokenType type = parser->current.type;
		parser->current = GetToken(lexer);

		AstNode *rhs = ParseExpression(parser, lexer, bindingPower);
		switch (type)
		{
			case TokenType_Plus:
			{
				lhs = MakeNode(NodeType_Add);
			} break;

			case TokenType_Minus:
			{
				lhs = MakeNode(NodeType_Subtract);
			} break;

			case TokenType_Asterisk:
			{
				lhs = MakeNode(NodeType_Multiply);
			} break;

			case TokenType_Slash:
			{
				lhs = MakeNode();
			} break;

			default: {} break;
		}
	}

	return lhs;
}
