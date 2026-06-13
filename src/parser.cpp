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
AdvanceToken(Parser *parser, Lexer *lexer)
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
		AdvanceToken(parser, lexer);
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

internal void
UnexpectedToken(Parser *parser)
{
	Error(parser, "unexpected token %s", GetTokenTypeName(parser->current.type));
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

	switch (parser->current.type)
	{
		case TokenType_Number:
		{
			node = MakeNumberNode(parser->current.line, parser->current.numberValue, arena);
			AdvanceToken(parser, lexer);
		} break;

		case TokenType_OpenParen:
		{
			AdvanceToken(parser, lexer);
			node = ParseExpression(parser, lexer, 0, arena);
			ExpectToken(parser, lexer, TokenType_CloseParen);
		} break;

		case TokenType_Minus:
		{
			// unary minus

			node = MakeNode(NodeType_Subtract, parser->current.line, arena);
			node->binary.lhs = MakeNumberNode(parser->current.line, 0, arena);
			AdvanceToken(parser, lexer);
			node->binary.rhs = ParseAtom(parser, lexer, arena);
		} break;

		case TokenType_Identifier:
		{
			if (PeekToken(lexer).type == TokenType_OpenParen)
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
					   && parser->current.type != TokenType_CloseParen)
				{
					if (node->call.numExpressions != 0)
					{
						ExpectToken(parser, lexer, TokenType_Comma);
					}

					Assert(node->call.numExpressions < MAX_ARGUMENTS);
					node->call.expressions[node->call.numExpressions] = ParseExpression(parser, lexer, 0, arena);

					node->call.numExpressions++;
				}

				ExpectToken(parser, lexer, TokenType_CloseParen);
			}
			else
			{
				// variable access

				node = MakeNode(NodeType_Var, parser->current.line, arena);
				node->var.name = parser->current.str;

				AdvanceToken(parser, lexer);
			}
		} break;

		default:
		{
			UnexpectedToken(parser);
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
		int bindingPower = LeftBindingPower(parser->current.type);
		if (bindingPower <= minBindingPower)
		{
			break;
		}

		TokenType type = parser->current.type;
		int line = parser->current.line;

		AdvanceToken(parser, lexer);

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

	if (!ExpectToken(parser, lexer, TokenType_OpenBrace))
	{
		return nullptr;
	}

	AstNode *block = MakeNode(NodeType_Block, openBraceTokenLine, arena);
	block->block.statements = PushArray(arena, MAX_STATEMENTS, AstNode *);
	block->block.numStatements = 0;

	while (parser->current.type != TokenType_CloseBrace
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

	if (!ExpectToken(parser, lexer, TokenType_CloseBrace))
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
		AdvanceToken(parser, lexer);

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
			AdvanceToken(parser, lexer);

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
		AdvanceToken(parser, lexer);

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
		AdvanceToken(parser, lexer);

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
	if (parser->current.type == TokenType_OpenBrace)
	{
		AstNode *block = ParseBlock(parser, lexer, arena);
		return block;
	}

	if (parser->current.type == TokenType_Return)
	{
		int returnTokenLine = parser->current.line;

		// eat the 'return'
		AdvanceToken(parser, lexer);

		AstNode *expr = ParseExpression(parser, lexer, 0, arena);	

		if (!ExpectToken(parser, lexer, TokenType_Semicolon))
		{
			return nullptr;
		}

		AstNode *node = MakeNode(NodeType_Return, returnTokenLine, arena);
		node->ret.expr = expr;

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

		string name = expr->var.name;

		// eat the colon
		AdvanceToken(parser, lexer);

		if (!ExpectToken(parser, lexer, TokenType_Identifier))
		{
			return nullptr;
		}
		
		if (parser->current.type == TokenType_Equal)
		{
			// name : type = expr;

			int equalTokenLine = parser->current.line;

			// eat the '='
			AdvanceToken(parser, lexer);

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
			AdvanceToken(parser, lexer);

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
		AdvanceToken(parser, lexer);

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

internal AstNode *
ParseTopLevelStatement(Parser *parser,
					   Lexer *lexer,
					   Arena *arena)
{
	// function definition
	// main :: proc(foo: int) { statements; }
	if (parser->current.type == TokenType_Identifier)
	{
		const int MAX_ARGUMENTS = 32;

		AstNode *node = MakeNode(NodeType_Func, parser->current.line, arena);
		node->func.name = parser->current.str;
		node->func.params = PushArray(arena, MAX_ARGUMENTS, Parameter);
		node->func.numParams = 0;

		// eat the function name
		AdvanceToken(parser, lexer);

		ExpectToken(parser, lexer, TokenType_Colon);
		ExpectToken(parser, lexer, TokenType_Colon);

		ExpectToken(parser, lexer, TokenType_Proc);

		ExpectToken(parser, lexer, TokenType_OpenParen);

		while (!parser->hadError
			   && parser->current.type != TokenType_CloseParen)
		{
			if (node->func.numParams != 0)
			{
				ExpectToken(parser, lexer, TokenType_Comma);
			}

			Assert(node->func.numParams < MAX_ARGUMENTS);
			node->func.params[node->func.numParams].name = parser->current.str;
			ExpectToken(parser, lexer, TokenType_Identifier);

			ExpectToken(parser, lexer, TokenType_Colon);

			ExpectToken(parser, lexer, TokenType_Identifier);

			node->func.numParams++;
		}

		ExpectToken(parser, lexer, TokenType_CloseParen);

		node->func.body = ParseBlock(parser, lexer, arena);

		return node;
	}

	Error(parser, "unexpected token %s", GetTokenTypeName(parser->current.type));

	return nullptr;
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
		   && parser->current.type != TokenType_EOF)
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
