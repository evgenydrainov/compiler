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

internal bool
ExpectToken(Parser *parser,
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
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower,
				Arena *arena);

internal AstNode *
ParseAtom(Parser *parser,
		  Lexer *lexer,
		  Arena *arena)
{
	if (parser->current.type == TokenType_Number)
	{
		AstNode *node = MakeNumberNode(parser->current.line, parser->current.numberValue, arena);

		NextToken(parser, lexer);

		return node;
	}

	if (parser->current.type == TokenType_LeftParen)
	{
		NextToken(parser, lexer);

		AstNode *node = ParseExpression(parser, lexer, 0, arena);

		ExpectToken(parser, lexer, TokenType_RightParen);

		return node;
	}

	if (parser->current.type == TokenType_Minus)
	{
		int line = parser->current.line;

		NextToken(parser, lexer);

		AstNode *lhs = MakeNumberNode(line, 0, arena);

		AstNode *rhs = ParseAtom(parser, lexer, arena);

		AstNode *node = MakeNode(NodeType_Subtract, line, arena);
		node->binary.lhs = lhs;
		node->binary.rhs = rhs;

		return node;
	}

	if (parser->current.type == TokenType_Identifier)
	{
		AstNode *node = MakeNode(NodeType_Var, parser->current.line, arena);
		node->var.name = parser->current.str;

		NextToken(parser, lexer);

		return node;
	}

	Error(parser, "unexpected token %s", GetTokenTypeName(parser->current.type));

	return nullptr;
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
		int bindingPower = LeftBindingPower(parser->current.type);
		if (bindingPower <= minBindingPower)
		{
			break;
		}

		TokenType type = parser->current.type;
		int line = parser->current.line;

		NextToken(parser, lexer);

		AstNode *rhs = ParseExpression(parser, lexer, bindingPower, arena);
		switch (type)
		{
			case TokenType_Plus:
			{
				lhs = MakeBinaryNode(NodeType_Add, line, lhs, rhs, arena);
			} break;

			case TokenType_Minus:
			{
				lhs = MakeBinaryNode(NodeType_Subtract, line, lhs, rhs, arena);
			} break;

			case TokenType_Asterisk:
			{
				lhs = MakeBinaryNode(NodeType_Multiply, line, lhs, rhs, arena);
			} break;

			case TokenType_Slash:
			{
				lhs = MakeBinaryNode(NodeType_Divide, line, lhs, rhs, arena);
			} break;

			case TokenType_Less:
			{
				lhs = MakeBinaryNode(NodeType_Less, line, lhs, rhs, arena);
			} break;

			case TokenType_Greater:
			{
				lhs = MakeBinaryNode(NodeType_Greater, line, lhs, rhs, arena);
			} break;

			case TokenType_EqualEqual:
			{
				lhs = MakeBinaryNode(NodeType_EqualEqual, line, lhs, rhs, arena);
			} break;

			case TokenType_LessEqual:
			{
				lhs = MakeBinaryNode(NodeType_LessEqual, line, lhs, rhs, arena);
			} break;

			case TokenType_GreaterEqual:
			{
				lhs = MakeBinaryNode(NodeType_GreaterEqual, line, lhs, rhs, arena);
			} break;

			case TokenType_BangEqual:
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

	if (!ExpectToken(parser, lexer, TokenType_LeftBrace))
	{
		return nullptr;
	}

	AstNode *block = MakeNode(NodeType_Block, openBraceTokenLine, arena);
	block->block.statements = PushArray(arena, MAX_STATEMENTS, AstNode *);
	block->block.numStatements = 0;

	while (parser->current.type != TokenType_RightBrace
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

	if (!ExpectToken(parser, lexer, TokenType_RightBrace))
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
		int ifTokenLine = parser->current.line;

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

		AstNode *node = MakeNode(NodeType_If, ifTokenLine, arena);
		node->_if.condition = condition;
		node->_if.thenBlock = thenBlock;
		node->_if.elseBlock = elseBlock;

		return node;
	}

	// while expr { statements; }
	if (parser->current.type == TokenType_While)
	{
		int whileTokenLine = parser->current.line;

		// eat the 'while'
		NextToken(parser, lexer);

		AstNode *condition = ParseExpression(parser, lexer, 0, arena);	

		if (parser->hadError)
		{
			return nullptr;
		}

		AstNode *loopBody = ParseBlock(parser, lexer, arena);

		AstNode *node = MakeNode(NodeType_While, whileTokenLine, arena);
		node->_while.condition = condition;
		node->_while.body = loopBody;

		return node;
	}

	// print expr;
	if (parser->current.type == TokenType_Print)
	{
		int printTokenLine = parser->current.line;

		// eat the 'print'
		NextToken(parser, lexer);

		AstNode *expr = ParseExpression(parser, lexer, 0, arena);	

		if (!ExpectToken(parser, lexer, TokenType_Semicolon))
		{
			return nullptr;
		}

		AstNode *node = MakeNode(NodeType_Print, printTokenLine, arena);
		node->print.expr = expr;

		return node;
	}

	// empty scope
	// { statements; }
	if (parser->current.type == TokenType_LeftBrace)
	{
		AstNode *block = ParseBlock(parser, lexer, arena);
		return block;
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

		string name = expr->var.name;

		// eat the colon
		NextToken(parser, lexer);

		if (!ExpectToken(parser, lexer, TokenType_Identifier))
		{
			return nullptr;
		}
		
		if (parser->current.type == TokenType_Equal)
		{
			// name : type = expr;

			int equalTokenLine = parser->current.line;

			// eat the '='
			NextToken(parser, lexer);

			AstNode *rhs = ParseExpression(parser, lexer, 0, arena);

			if (!ExpectToken(parser, lexer, TokenType_Semicolon))
			{
				return nullptr;
			}

			AstNode *node = MakeNode(NodeType_VarDecl, equalTokenLine, arena);
			node->assign.expr = rhs;
			node->assign.name = name;

			return node;
		}
		else if (parser->current.type == TokenType_Semicolon)
		{
			// name : type;

			int semicolonTokenLine = parser->current.line;

			// eat the semicolon
			NextToken(parser, lexer);

			// initialize to zero by default
			AstNode *rhs = MakeNumberNode(semicolonTokenLine, 0, arena);

			AstNode *node = MakeNode(NodeType_VarDecl, semicolonTokenLine, arena);
			node->assign.expr = rhs;
			node->assign.name = name;

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

		string name = expr->var.name;

		int equalTokenLine = parser->current.line;

		// eat the '='
		NextToken(parser, lexer);

		AstNode *rhs = ParseExpression(parser, lexer, 0, arena);

		AstNode *node = MakeNode(NodeType_Assign, equalTokenLine, arena);
		node->assign.expr = rhs;
		node->assign.name = name;

		if (!ExpectToken(parser, lexer, TokenType_Semicolon))
		{
			return nullptr;
		}

		return node;
	}

	// bare expression
	// expr;
	if (!ExpectToken(parser, lexer, TokenType_Semicolon))
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
