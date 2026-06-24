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
		case TokenKind_Percent:
			return 3;

		default:
			return 0;
	}
}

template <typename T>
internal T *
MakeNode(NodeKind kind,
		 int line,
		 Arena *arena)
{
	T *node = PushStruct<T>(arena);
	node->kind = kind;
	node->line = line;

	return node;
}

internal BinaryNode *
MakeBinaryNode(NodeKind type,
			   int line,
			   Node *lhs,
			   Node *rhs,
			   Arena *arena)
{
	BinaryNode *node = MakeNode<BinaryNode>(type, line, arena);
	node->lhs = lhs;
	node->rhs = rhs;

	return node;
}

internal NumberNode *
MakeNumberNode(int line,
			   int numberValue,
			   Arena *arena)
{
	NumberNode *node = MakeNode<NumberNode>(NodeKind_Number, line, arena);
	node->int64Value = numberValue;

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

internal Node *
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower,
				Arena *arena);

internal Node *
ParseAtom(Parser *parser,
		  Lexer *lexer,
		  Arena *arena)
{
	Node *result = nullptr;

	switch (parser->current.kind)
	{
		case TokenKind_Number:
		{
			NumberNode *node = MakeNumberNode(parser->current.line, parser->current.numberValue, arena);
			AdvanceToken(parser, lexer);

			result = node;
		} break;

		case TokenKind_OpenParen:
		{
			AdvanceToken(parser, lexer);
			Node *node = ParseExpression(parser, lexer, 0, arena);
			ExpectToken(parser, lexer, TokenKind_CloseParen);

			result = node;
		} break;

		case TokenKind_Minus:
		{
			// unary minus

			BinaryNode *node = MakeNode<BinaryNode>(NodeKind_Subtract, parser->current.line, arena);
			node->lhs = MakeNumberNode(parser->current.line, 0, arena);
			AdvanceToken(parser, lexer);
			node->rhs = ParseAtom(parser, lexer, arena);

			result = node;
		} break;

		case TokenKind_Identifier:
		{
			if (PeekToken(lexer).kind == TokenKind_OpenParen)
			{
				// function call
				// foo(a, b, c)

				const int MAX_ARGUMENTS = 32;

				CallNode *node = MakeNode<CallNode>(NodeKind_Call, parser->current.line, arena);
				node->name = parser->current.str;
				node->expressions = PushArray<Node *>(arena, MAX_ARGUMENTS);
				node->numExpressions = 0;

				AdvanceToken(parser, lexer); // eat the function name
				AdvanceToken(parser, lexer); // eat the '('

				while (!parser->hadError
					   && parser->current.kind != TokenKind_CloseParen)
				{
					if (node->numExpressions != 0)
					{
						ExpectToken(parser, lexer, TokenKind_Comma);
					}

					Assert(node->numExpressions < MAX_ARGUMENTS);
					node->expressions[node->numExpressions] = ParseExpression(parser, lexer, 0, arena);

					node->numExpressions++;
				}

				ExpectToken(parser, lexer, TokenKind_CloseParen);

				result = node;
			}
			else
			{
				// variable access

				VarNode *node = MakeNode<VarNode>(NodeKind_Var, parser->current.line, arena);
				node->name = parser->current.str;

				AdvanceToken(parser, lexer);

				result = node;
			}
		} break;

		case TokenKind_True:
		{
			BoolNode *node = MakeNode<BoolNode>(NodeKind_Bool, parser->current.line, arena);
			node->boolValue = true;
			AdvanceToken(parser, lexer);

			result = node;
		} break;

		case TokenKind_False:
		{
			BoolNode *node = MakeNode<BoolNode>(NodeKind_Bool, parser->current.line, arena);
			node->boolValue = false;
			AdvanceToken(parser, lexer);

			result = node;
		} break;

		case TokenKind_Ampersand:
		{
			AddressOfNode *node = MakeNode<AddressOfNode>(NodeKind_AddressOf, parser->current.line, arena);
			AdvanceToken(parser, lexer);

			node->what = ParseAtom(parser, lexer, arena);

			result = node;
		} break;

		case TokenKind_Asterisk:
		{
			DerefNode *node = MakeNode<DerefNode>(NodeKind_Deref, parser->current.line, arena);
			AdvanceToken(parser, lexer);

			node->what = ParseAtom(parser, lexer, arena);

			result = node;
		} break;

		default:
		{
			UnexpectedCurrentToken(parser);
		} break;
	}

	if (parser->current.kind == TokenKind_Dot)
	{
		FieldAccessNode *node = MakeNode<FieldAccessNode>(NodeKind_FieldAccess, parser->current.line, arena);
		node->expr = result;

		AdvanceToken(parser, lexer); // eat the '.'

		node->fieldName = parser->current.str;

		ExpectToken(parser, lexer, TokenKind_Identifier);

		result = node;
	}
	
	return result;
}

internal Node *
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower,
				Arena *arena)
{
	Node *lhs = ParseAtom(parser, lexer, arena);
	for (;;)
	{
		int bindingPower = LeftBindingPower(parser->current.kind);
		if (bindingPower <= minBindingPower)
		{
			break;
		}

		TokenKind kind = parser->current.kind;
		int line = parser->current.line;

		AdvanceToken(parser, lexer);

		Node *rhs = ParseExpression(parser, lexer, bindingPower, arena);
		switch (kind)
		{
			case TokenKind_Plus:
			{
				lhs = MakeBinaryNode(NodeKind_Add, line, lhs, rhs, arena);
			} break;

			case TokenKind_Minus:
			{
				lhs = MakeBinaryNode(NodeKind_Subtract, line, lhs, rhs, arena);
			} break;

			case TokenKind_Asterisk:
			{
				lhs = MakeBinaryNode(NodeKind_Multiply, line, lhs, rhs, arena);
			} break;

			case TokenKind_Slash:
			{
				lhs = MakeBinaryNode(NodeKind_Divide, line, lhs, rhs, arena);
			} break;

			case TokenKind_Percent:
			{
				lhs = MakeBinaryNode(NodeKind_Modulo, line, lhs, rhs, arena);
			} break;

			case TokenKind_Less:
			{
				lhs = MakeBinaryNode(NodeKind_Less, line, lhs, rhs, arena);
			} break;

			case TokenKind_Greater:
			{
				lhs = MakeBinaryNode(NodeKind_Greater, line, lhs, rhs, arena);
			} break;

			case TokenKind_EqualEqual:
			{
				lhs = MakeBinaryNode(NodeKind_EqualEqual, line, lhs, rhs, arena);
			} break;

			case TokenKind_LessEqual:
			{
				lhs = MakeBinaryNode(NodeKind_LessEqual, line, lhs, rhs, arena);
			} break;

			case TokenKind_GreaterEqual:
			{
				lhs = MakeBinaryNode(NodeKind_GreaterEqual, line, lhs, rhs, arena);
			} break;

			case TokenKind_BangEqual:
			{
				lhs = MakeBinaryNode(NodeKind_NotEqual, line, lhs, rhs, arena);
			} break;

			default: {} break;
		}
	}

	return lhs;
}

internal Node *
ParseStatement(Parser *parser,
			   Lexer *lexer,
			   Arena *arena);

internal Node *
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

	BlockNode *block = MakeNode<BlockNode>(NodeKind_Block, openBraceTokenLine, arena);
	block->statements = PushArray<Node *>(arena, MAX_STATEMENTS);
	block->numStatements = 0;

	while (parser->current.kind != TokenKind_CloseBrace
		   && !parser->hadError)
	{
		Node *statement = ParseStatement(parser, lexer, arena);
		if (parser->hadError)
		{
			break;
		}

		if (statement)
		{
			Assert(block->numStatements < MAX_STATEMENTS);
			block->statements[block->numStatements++] = statement;
		}
		else
		{
			// it's an empty statement - ignore it
		}
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

		type.pointerTo = PushStruct<Type>(arena);
		*type.pointerTo = ParseType(parser, lexer, arena);
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
	else if (parser->current.kind == TokenKind_Identifier)
	{
		type.kind = TypeKind_Struct;
		type.name = parser->current.str;

		AdvanceToken(parser, lexer);
	}
	else
	{
		UnexpectedCurrentToken(parser);
	}

	return type;
}

internal Node *
ParseReturnStatement(Parser *parser,
					 Lexer *lexer,
					 Arena *arena)
{
	ReturnNode *node = MakeNode<ReturnNode>(NodeKind_Return, parser->current.line, arena);

	// eat the 'return'
	AdvanceToken(parser, lexer);

	if (parser->current.kind == TokenKind_Semicolon)
	{
		// bare return;
	}
	else
	{
		// return expr;

		node->expr = ParseExpression(parser, lexer, 0, arena);	
	}

	ExpectToken(parser, lexer, TokenKind_Semicolon);

	return node;
}

internal Node *
ParseIfStatement(Parser *parser,
				 Lexer *lexer,
				 Arena *arena)
{
	// if expr { statements; }

	IfNode *node = MakeNode<IfNode>(NodeKind_If, parser->current.line, arena);

	// eat the 'if'
	AdvanceToken(parser, lexer);

	node->condition = ParseExpression(parser, lexer, 0, arena);	

	node->thenBlock = ParseBlock(parser, lexer, arena);

	if (parser->current.kind == TokenKind_Else)
	{
		// eat the 'else'
		AdvanceToken(parser, lexer);

		node->elseBlock = ParseBlock(parser, lexer, arena);
	}

	return node;
}

internal Node *
ParseWhileStatement(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	// while expr { statements; }

	WhileNode *node = MakeNode<WhileNode>(NodeKind_While, parser->current.line, arena);

	// eat the 'while'
	AdvanceToken(parser, lexer);

	node->condition = ParseExpression(parser, lexer, 0, arena);	

	node->body = ParseBlock(parser, lexer, arena);

	return node;
}

internal Node *
ParsePrintStatement(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	// print expr;

	PrintNode *node = MakeNode<PrintNode>(NodeKind_Print, parser->current.line, arena);

	// eat the 'print'
	AdvanceToken(parser, lexer);

	node->expr = ParseExpression(parser, lexer, 0, arena);	

	ExpectToken(parser, lexer, TokenKind_Semicolon);

	return node;
}

internal Node *
ParseVariableDeclaration(Parser *parser,
						 Lexer *lexer,
						 Arena *arena)
{
	// variable declaration

	VarDeclNode *node = MakeNode<VarDeclNode>(NodeKind_VarDecl, parser->current.line, arena);
	node->name = parser->current.str;

	// eat the variable name
	AdvanceToken(parser, lexer);

	ExpectToken(parser, lexer, TokenKind_Colon);

	node->type = ParseType(parser, lexer, arena);

	if (parser->current.kind == TokenKind_Equal)
	{
		// name : type = expr;

		// eat the '='
		AdvanceToken(parser, lexer);

		node->expr = ParseExpression(parser, lexer, 0, arena);

		ExpectToken(parser, lexer, TokenKind_Semicolon);
	}
	else if (parser->current.kind == TokenKind_Semicolon)
	{
		// name : type;

		// eat the semicolon
		AdvanceToken(parser, lexer);

		// initialize to zero by default
		// node->expr = MakeNumberNode(parser->current.line, 0, arena);
	}
	else
	{
		ErrorAtCurrent(parser, "expected '=' or ';'");
	}

	return node;
}

internal Node *
ParseStatement(Parser *parser,
			   Lexer *lexer,
			   Arena *arena)
{
	if (parser->current.kind == TokenKind_If)
	{
		return ParseIfStatement(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_While)
	{
		return ParseWhileStatement(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Print)
	{
		return ParsePrintStatement(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_OpenBrace)
	{
		// empty scope
		// { statements; }
		return ParseBlock(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Return)
	{
		return ParseReturnStatement(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Identifier)
	{
		Token peekToken = PeekToken(lexer);
		if (peekToken.kind == TokenKind_Colon)
		{
			return ParseVariableDeclaration(parser, lexer, arena);
		}
	}

	if (parser->current.kind == TokenKind_Semicolon)
	{
		// bare semicolon - empty statement
		AdvanceToken(parser, lexer);
		return nullptr;
	}

	Node *expr = ParseExpression(parser, lexer, 0, arena);
	if (parser->current.kind == TokenKind_Equal)
	{
		// assignment
		// lhs = rhs;

		AssignNode *node = MakeNode<AssignNode>(NodeKind_Assign, parser->current.line, arena);
		node->lhs = expr;

		AdvanceToken(parser, lexer); // eat the '='

		node->rhs = ParseExpression(parser, lexer, 0, arena);

		ExpectToken(parser, lexer, TokenKind_Semicolon);

		return node;
	}

	// bare expression
	// expr;
	ExpectToken(parser, lexer, TokenKind_Semicolon);
	return expr;
}

internal Node *
ParseFunctionDefinition(Parser *parser,
						Lexer *lexer,
						Arena *arena)
{
	// function definition
	// main :: proc(foo: int) -> i64 { statements; }

	const int MAX_ARGUMENTS = 32;

	FuncNode *node = MakeNode<FuncNode>(NodeKind_Func, parser->current.line, arena);
	node->name = parser->current.str;
	node->params = PushArray<Node *>(arena, MAX_ARGUMENTS);
	node->numParams = 0;

	// eat the function name
	AdvanceToken(parser, lexer);

	ExpectToken(parser, lexer, TokenKind_Colon);
	ExpectToken(parser, lexer, TokenKind_Colon);

	ExpectToken(parser, lexer, TokenKind_Proc);

	ExpectToken(parser, lexer, TokenKind_OpenParen);

	while (!parser->hadError
			&& parser->current.kind != TokenKind_CloseParen)
	{
		if (node->numParams != 0)
		{
			ExpectToken(parser, lexer, TokenKind_Comma);
		}

		ParamNode *param = MakeNode<ParamNode>(NodeKind_Param, parser->current.line, arena);
		param->name = parser->current.str;

		ExpectToken(parser, lexer, TokenKind_Identifier); // eat the param name

		ExpectToken(parser, lexer, TokenKind_Colon);

		param->type = ParseType(parser, lexer, arena);

		Assert(node->numParams < MAX_ARGUMENTS);
		node->params[node->numParams] = param;

		node->numParams++;
	}

	AdvanceToken(parser, lexer); // eat the ')'

	node->returnType.kind = TypeKind_Void;

	if (parser->current.kind == TokenKind_Arrow)
	{
		AdvanceToken(parser, lexer); // eat the '->'

		node->returnType = ParseType(parser, lexer, arena);
	}

	node->body = ParseBlock(parser, lexer, arena);

	return node;
}

internal Node *
ParseStructDefinition(Parser *parser,
					  Lexer *lexer,
					  Arena *arena)
{
	const int MAX_STRUCT_FIELDS = 32;

	StructDeclNode *node = MakeNode<StructDeclNode>(NodeKind_StructDecl, parser->current.line, arena);
	node->name = parser->current.str;
	node->fields = PushBumpArray<StructFieldDeclNode *>(arena, MAX_STRUCT_FIELDS);

	AdvanceToken(parser, lexer); // eat the struct name

	ExpectToken(parser, lexer, TokenKind_Colon);
	ExpectToken(parser, lexer, TokenKind_Colon);

	ExpectToken(parser, lexer, TokenKind_Struct);

	ExpectToken(parser, lexer, TokenKind_OpenBrace);

	while (!parser->hadError
		   && parser->current.kind != TokenKind_CloseBrace)
	{
		StructFieldDeclNode *field = MakeNode<StructFieldDeclNode>(NodeKind_StructFieldDecl, parser->current.line, arena);
		field->name = parser->current.str;

		ExpectToken(parser, lexer, TokenKind_Identifier);

		ExpectToken(parser, lexer, TokenKind_Colon);

		field->type = ParseType(parser, lexer, arena);

		ExpectToken(parser, lexer, TokenKind_Semicolon);

		ArrayAdd(&node->fields, field);
	}

	ExpectToken(parser, lexer, TokenKind_CloseBrace);

	return node;
}

internal Node *
ParseTopLevelStatement(Parser *parser,
					   Lexer *lexer,
					   Arena *arena)
{
	if (parser->current.kind == TokenKind_Identifier)
	{
		Token peekToken0 = PeekToken(lexer, 1);
		Token peekToken1 = PeekToken(lexer, 2);
		Token peekToken2 = PeekToken(lexer, 3);

		if (peekToken0.kind == TokenKind_Colon
			&& peekToken1.kind == TokenKind_Colon
			&& peekToken2.kind == TokenKind_Proc)
		{
			return ParseFunctionDefinition(parser, lexer, arena);
		}

		if (peekToken0.kind == TokenKind_Colon
			&& peekToken1.kind == TokenKind_Colon
			&& peekToken2.kind == TokenKind_Struct)
		{
			return ParseStructDefinition(parser, lexer, arena);
		}
	}

	UnexpectedCurrentToken(parser);

	return nullptr;
}

Node *
ParseProgram(Parser *parser,
			 Lexer *lexer,
			 Arena *arena)
{
	const int MAX_STATEMENTS = 1024;

	BlockNode *block = MakeNode<BlockNode>(NodeKind_Block, 1, arena);
	block->statements = PushArray<Node *>(arena, MAX_STATEMENTS);
	block->numStatements = 0;

	while (!parser->hadError
		   && parser->current.kind != TokenKind_EOF)
	{
		Node *statement = ParseTopLevelStatement(parser, lexer, arena);
		if (parser->hadError)
		{
			break;
		}

		Assert(block->numStatements < MAX_STATEMENTS);
		block->statements[block->numStatements++] = statement;
	}

	return block;
}
