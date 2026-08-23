#include "parser.h"
#include <stdio.h>
#include <stdarg.h>

internal int
LeftBindingPower(TokenKind type)
{
	switch (type)
	{
		case TokenKind_PipePipe:
			return 1;

		case TokenKind_AmpAmp:
			return 2;

		case TokenKind_EqualEqual:
		case TokenKind_BangEqual:
			return 3;

		case TokenKind_Greater:
		case TokenKind_Less:
		case TokenKind_GreaterEqual:
		case TokenKind_LessEqual:
			return 4;

		case TokenKind_Pipe:
			return 5;

		case TokenKind_Caret:
			return 6;

		case TokenKind_Ampersand:
			return 7;

		case TokenKind_Plus:
		case TokenKind_Minus:
			return 8;

		case TokenKind_GreaterGreater:
		case TokenKind_LessLess:
			return 9;

		case TokenKind_Star:
		case TokenKind_Slash:
		case TokenKind_Percent:
			return 10;

		default:
			return 0;
	}
}

internal void
ErrorAtTokenVA(Parser *parser,
			   Token token,
			   char *format,
			   va_list args)
{
	fprintf(stderr, STR_FMT "(%d, %d): error: ",
			STR_ARG(token.location.fileName), token.location.line, token.location.column);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");

	parser->hadError = true;
}

internal void
ErrorAtToken(Parser *parser,
			 Token token,
			 char *format,
			 ...)
{
	va_list args;
	va_start(args, format);
	ErrorAtTokenVA(parser, token, format, args);
	va_end(args);
}

internal void
ErrorAtCurrent(Parser *parser,
			   char *format,
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

internal void
ExpectToken(Parser *parser,
			Lexer *lexer,
			TokenKind kind)
{
	bool insertedSemicolon = false;

	if (kind == TokenKind_Semicolon
		&& parser->numInsertSemicolons > 0)
	{
		parser->numInsertSemicolons--;
		insertedSemicolon = true;
	}

	if (parser->current.kind == kind)
	{
		AdvanceToken(parser, lexer);
	}
	else
	{
		if (!insertedSemicolon)
		{
			if (parser->current.kind == TokenKind_Error)
			{
				string errorMessage = parser->current.str;
				ErrorAtCurrent(parser, STR_FMT, STR_ARG(errorMessage));
			}
			else
			{
				ErrorAtCurrent(parser,
							   "expected '%s', but got '%s'",
							   GetTokenKindName(kind),
							   GetTokenKindName(parser->current.kind));
			}
		}
	}
}

internal void
UnexpectedCurrentToken(Parser *parser)
{
	if (parser->current.kind == TokenKind_Error)
	{
		string errorMessage = parser->current.str;
		ErrorAtCurrent(parser, STR_FMT, STR_ARG(errorMessage));
	}
	else
	{
		ErrorAtCurrent(parser, "unexpected token '%s'", GetTokenKindName(parser->current.kind));
	}
}

internal Node *
ParseExpression(Parser *parser,
				Lexer *lexer,
				int minBindingPower,
				Arena *arena);

internal Type
ParseType(Parser *parser,
		  Lexer *lexer,
		  Arena *arena)
{
	if (parser->current.kind == TokenKind_Star)
	{
		Type type = {};
		type.kind = TypeKind_Pointer;
		type.pointerTo = PushStruct<Type>(arena);

		AdvanceToken(parser, lexer); // eat the '*'

		*type.pointerTo = ParseType(parser, lexer, arena);

		return type;
	}

	if (parser->current.kind == TokenKind_OpenBracket)
	{
		AdvanceToken(parser, lexer); // eat the '['

		if (parser->current.kind == TokenKind_CloseBracket)
		{
			Type type = {};
			type.kind = TypeKind_Slice;
			type.arrayElementType = PushStruct<Type>(arena);

			ExpectToken(parser, lexer, TokenKind_CloseBracket);

			*type.arrayElementType = ParseType(parser, lexer, arena);

			return type;
		}

		if (parser->current.kind == TokenKind_DotDot)
		{
			AdvanceToken(parser, lexer); // eat the '..'

			Type type = {};
			type.kind = TypeKind_DynamicArray;
			type.arrayElementType = PushStruct<Type>(arena);

			ExpectToken(parser, lexer, TokenKind_CloseBracket);

			*type.arrayElementType = ParseType(parser, lexer, arena);

			return type;
		}

		Type type = {};
		type.kind = TypeKind_Array;
		type.arrayElementType = PushStruct<Type>(arena);
		type.arrayLengthExpr = ParseExpression(parser, lexer, 0, arena);

		ExpectToken(parser, lexer, TokenKind_CloseBracket);

		*type.arrayElementType = ParseType(parser, lexer, arena);

		return type;
	}

	switch (parser->current.str[0])
	{
		case 'i':
		{
			if (parser->current.str == "i8")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Int8 };
			}

			if (parser->current.str == "i16")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Int16 };
			}

			if (parser->current.str == "i32")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Int32 };
			}

			if (parser->current.str == "i64")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Int64 };
			}

			if (parser->current.str == "int")
			{
				// 'int' is alias for 'i64'
				AdvanceToken(parser, lexer);
				return { TypeKind_Int64 };
			}
		} break;

		case 'u':
		{
			if (parser->current.str == "u8")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_UInt8 };
			}

			if (parser->current.str == "u16")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_UInt16 };
			}

			if (parser->current.str == "u32")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_UInt32 };
			}

			if (parser->current.str == "u64")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_UInt64 };
			}
		} break;

		case 'f':
		{
			if (parser->current.str == "f32")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Float32 };
			}

			if (parser->current.str == "f64")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Float64 };
			}
		} break;

		case 'b':
		{
			if (parser->current.str == "bool")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Bool };
			}
		} break;

		case 'v':
		{
			if (parser->current.str == "void")
			{
				AdvanceToken(parser, lexer);
				return { TypeKind_Void };
			}
		} break;
	}
	
	if (parser->current.kind == TokenKind_Identifier)
	{
		Type type = {};
		type.kind = TypeKind_Struct;
		type.name = parser->current.str;

		AdvanceToken(parser, lexer);

		return type;
	}

	UnexpectedCurrentToken(parser);
	return {};
}

internal Node *
ParseAtom(Parser *parser,
		  Lexer *lexer,
		  Arena *arena);

internal Node *
ParseAtom_Inner(Parser *parser,
				Lexer *lexer,
				Arena *arena)
{
	if (parser->current.kind == TokenKind_Int64Literal)
	{
		Int64LiteralNode *node = MakeInt64Literal(parser->current.location, parser->current.int64Value, arena);
		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_Float32Literal)
	{
		Float32LiteralNode *node = MakeNode<Float32LiteralNode>(parser->current.location, arena);
		node->value = parser->current.float32Value;

		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_Null)
	{
		NullLiteralNode *node = MakeNode<NullLiteralNode>(parser->current.location, arena);
		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_Float64Literal)
	{
		Float64LiteralNode *node = MakeNode<Float64LiteralNode>(parser->current.location, arena);
		node->value = parser->current.float64Value;

		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_String)
	{
		StringNode *node = MakeNode<StringNode>(parser->current.location, arena);

		Assert(parser->current.str.count >= 2);
		node->value.data = parser->current.str.data + 1;
		node->value.count = parser->current.str.count - 2;
		node->uniqueId = parser->uniqueLabelId++;

		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_CString)
	{
		CStringNode *node = MakeNode<CStringNode>(parser->current.location, arena);

		Assert(parser->current.str.count >= 3);
		node->value.data = parser->current.str.data + 1;
		node->value.count = parser->current.str.count - 3;
		node->uniqueId = parser->uniqueLabelId++;

		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_OpenParen)
	{
		AdvanceToken(parser, lexer);
		Node *node = ParseExpression(parser, lexer, 0, arena);
		ExpectToken(parser, lexer, TokenKind_CloseParen);

		return node;
	}
	
	if (parser->current.kind == TokenKind_Minus)
	{
		// unary minus
		// -foo

		UnaryNode *node = MakeNode<UnaryNode>(parser->current.location, arena);
		node->op = UnaryOp_Negate;

		AdvanceToken(parser, lexer);

		node->expr = ParseAtom(parser, lexer, arena);

		return node;
	}

	if (parser->current.kind == TokenKind_Bang)
	{
		// logical not
		// !foo

		UnaryNode *node = MakeNode<UnaryNode>(parser->current.location, arena);
		node->op = UnaryOp_LogicalNot;

		AdvanceToken(parser, lexer);

		node->expr = ParseAtom(parser, lexer, arena);

		return node;
	}

	if (parser->current.kind == TokenKind_Tilde)
	{
		// bit-negate
		// ~foo

		UnaryNode *node = MakeNode<UnaryNode>(parser->current.location, arena);
		node->op = UnaryOp_BitNegate;

		AdvanceToken(parser, lexer);

		node->expr = ParseAtom(parser, lexer, arena);

		return node;
	}
			
	if (parser->current.kind == TokenKind_Identifier)
	{
		if (PeekToken(lexer).kind == TokenKind_OpenParen)
		{
			// function call
			// foo(a, b, c)

			const int MAX_ARGUMENTS = 32;

			CallNode *node = MakeNode<CallNode>(parser->current.location, arena);
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

			return node;
		}
		else
		{
			// variable access

			VarNode *node = MakeNode<VarNode>(parser->current.location, arena);
			node->name = parser->current.str;

			AdvanceToken(parser, lexer);

			return node;
		}
	}

	if (parser->current.kind == TokenKind_True)
	{
		BoolNode *node = MakeNode<BoolNode>(parser->current.location, arena);
		node->boolValue = true;
		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_False)
	{
		BoolNode *node = MakeNode<BoolNode>(parser->current.location, arena);
		node->boolValue = false;
		AdvanceToken(parser, lexer);

		return node;
	}

	if (parser->current.kind == TokenKind_Ampersand)
	{
		AddressOfNode *node = MakeNode<AddressOfNode>(parser->current.location, arena);
		AdvanceToken(parser, lexer);

		node->what = ParseAtom(parser, lexer, arena);

		return node;
	}

	if (parser->current.kind == TokenKind_Star)
	{
		DerefNode *node = MakeNode<DerefNode>(parser->current.location, arena);
		AdvanceToken(parser, lexer);

		node->what = ParseAtom(parser, lexer, arena);

		return node;
	}

	if (parser->current.kind == TokenKind_Cast)
	{
		CastNode *node = MakeNode<CastNode>(parser->current.location, arena);
		AdvanceToken(parser, lexer);

		ExpectToken(parser, lexer, TokenKind_OpenParen);

		node->targetType = ParseType(parser, lexer, arena);

		ExpectToken(parser, lexer, TokenKind_CloseParen);

		node->what = ParseAtom(parser, lexer, arena);

		return node;
	}

	UnexpectedCurrentToken(parser);

	return nullptr;
}

internal Node *
ParseAtom(Parser *parser,
		  Lexer *lexer,
		  Arena *arena)
{
	Node *result = nullptr;
	
	if (parser->current.kind != TokenKind_Dot)
	{
		result = ParseAtom_Inner(parser, lexer, arena);
	}

	while (parser->current.kind == TokenKind_Dot
		   || parser->current.kind == TokenKind_OpenBracket)
	{
		if (parser->current.kind == TokenKind_Dot)
		{
			FieldAccessNode *node = MakeNode<FieldAccessNode>(parser->current.location, arena);
			node->expr = result;

			AdvanceToken(parser, lexer); // eat the '.'

			node->fieldName = parser->current.str;

			ExpectToken(parser, lexer, TokenKind_Identifier);

			result = node;
		}
		else if (parser->current.kind == TokenKind_OpenBracket)
		{
			ArrayIndexAccessNode *node = MakeNode<ArrayIndexAccessNode>(parser->current.location, arena);
			node->arrayExpr = result;

			AdvanceToken(parser, lexer); // eat the '['

			node->indexExpr = ParseExpression(parser, lexer, 0, arena);

			ExpectToken(parser, lexer, TokenKind_CloseBracket);

			result = node;
		}
		else
		{
			Assert(false);
		}
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
		SourceLocation location = parser->current.location;

		AdvanceToken(parser, lexer);

		Node *rhs = ParseExpression(parser, lexer, bindingPower, arena);
		switch (kind)
		{
			default:
			{
				Assert(false);
			} break;

			case TokenKind_Plus:           {lhs = MakeBinaryNode(BinaryOp_Add,          location, lhs, rhs, arena);} break;
			case TokenKind_Minus:          {lhs = MakeBinaryNode(BinaryOp_Subtract,     location, lhs, rhs, arena);} break;
			case TokenKind_Star:           {lhs = MakeBinaryNode(BinaryOp_Multiply,     location, lhs, rhs, arena);} break;
			case TokenKind_Slash:          {lhs = MakeBinaryNode(BinaryOp_Divide,       location, lhs, rhs, arena);} break;
			case TokenKind_Percent:        {lhs = MakeBinaryNode(BinaryOp_Modulo,       location, lhs, rhs, arena);} break;
			case TokenKind_Less:           {lhs = MakeBinaryNode(BinaryOp_Less,         location, lhs, rhs, arena);} break;
			case TokenKind_Greater:        {lhs = MakeBinaryNode(BinaryOp_Greater,      location, lhs, rhs, arena);} break;
			case TokenKind_EqualEqual:     {lhs = MakeBinaryNode(BinaryOp_EqualEqual,   location, lhs, rhs, arena);} break;
			case TokenKind_LessEqual:      {lhs = MakeBinaryNode(BinaryOp_LessEqual,    location, lhs, rhs, arena);} break;
			case TokenKind_GreaterEqual:   {lhs = MakeBinaryNode(BinaryOp_GreaterEqual, location, lhs, rhs, arena);} break;
			case TokenKind_BangEqual:      {lhs = MakeBinaryNode(BinaryOp_NotEqual,     location, lhs, rhs, arena);} break;
			case TokenKind_AmpAmp:         {lhs = MakeBinaryNode(BinaryOp_LogicalAnd,   location, lhs, rhs, arena);} break;
			case TokenKind_PipePipe:       {lhs = MakeBinaryNode(BinaryOp_LogicalOr,    location, lhs, rhs, arena);} break;
			case TokenKind_GreaterGreater: {lhs = MakeBinaryNode(BinaryOp_ShiftRight,   location, lhs, rhs, arena);} break;
			case TokenKind_LessLess:       {lhs = MakeBinaryNode(BinaryOp_ShiftLeft,    location, lhs, rhs, arena);} break;
			case TokenKind_Ampersand:      {lhs = MakeBinaryNode(BinaryOp_BitAnd,       location, lhs, rhs, arena);} break;
			case TokenKind_Pipe:           {lhs = MakeBinaryNode(BinaryOp_BitOr,        location, lhs, rhs, arena);} break;
			case TokenKind_Caret:          {lhs = MakeBinaryNode(BinaryOp_BitXor,       location, lhs, rhs, arena);} break;
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

	SourceLocation openBraceTokenLocation = parser->current.location;

	ExpectToken(parser, lexer, TokenKind_OpenBrace);

	BlockNode *block = MakeNode<BlockNode>(openBraceTokenLocation, arena);
	block->statements = PushBumpArray<Node *>(arena, MAX_STATEMENTS);

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
			array_add(&block->statements, statement);
		}
		else
		{
			// it's an empty statement - ignore it
		}
	}

	ExpectToken(parser, lexer, TokenKind_CloseBrace);

	return block;
}

internal Node *
ParseReturnStatement(Parser *parser,
					 Lexer *lexer,
					 Arena *arena)
{
	ReturnNode *node = MakeNode<ReturnNode>(parser->current.location, arena);

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
ParseBlockOrSingleStatement(Parser *parser,
							Lexer *lexer,
							Arena *arena)
{
	if (parser->current.kind == TokenKind_OpenBrace)
	{
		return ParseBlock(parser, lexer, arena);
	}

	return ParseStatement(parser, lexer, arena);
}

internal Node *
ParseIfStatement(Parser *parser,
				 Lexer *lexer,
				 Arena *arena)
{
	// if expr { statements; }

	IfNode *node = MakeNode<IfNode>(parser->current.location, arena);

	// eat the 'if'
	AdvanceToken(parser, lexer);

	node->condition = ParseExpression(parser, lexer, 0, arena);	

	node->thenBlock = ParseBlockOrSingleStatement(parser, lexer, arena);

	if (parser->current.kind == TokenKind_Else)
	{
		// eat the 'else'
		AdvanceToken(parser, lexer);

		node->elseBlock = ParseBlockOrSingleStatement(parser, lexer, arena);
	}

	return node;
}

internal Node *
ParseWhileStatement(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	// while expr { statements; }

	WhileNode *node = MakeNode<WhileNode>(parser->current.location, arena);

	// eat the 'while'
	AdvanceToken(parser, lexer);

	node->condition = ParseExpression(parser, lexer, 0, arena);	

	node->body = ParseBlock(parser, lexer, arena);

	return node;
}

internal Node *
ParseForStatement(Parser *parser,
				  Lexer *lexer,
				  Arena *arena)
{
	// for init; cond; incr { statements; }

	ForNode *node = MakeNode<ForNode>(parser->current.location, arena);

	// eat the 'for'
	AdvanceToken(parser, lexer);

	node->init = ParseStatement(parser, lexer, arena);

	node->cond = ParseExpression(parser, lexer, 0, arena);
	ExpectToken(parser, lexer, TokenKind_Semicolon);

	parser->numInsertSemicolons++;
	node->incr = ParseStatement(parser, lexer, arena);

	node->body = ParseBlock(parser, lexer, arena);

	return node;
}

internal Node *
ParseForeachStatement(Parser *parser,
					  Lexer *lexer,
					  Arena *arena)
{
	BlockNode *blockNode = MakeNode<BlockNode>(parser->current.location, arena);
	blockNode->statements = PushBumpArray<Node *>(arena, 2);

	AdvanceToken(parser, lexer); // eat the 'for'

	bool iterateByPointer = false;

	if (parser->current.kind == TokenKind_Star)
	{
		AdvanceToken(parser, lexer); // eat the '*'
		iterateByPointer = true;
	}

	string iteratorName = parser->current.str;
	ExpectToken(parser, lexer, TokenKind_Identifier);

	ExpectToken(parser, lexer, TokenKind_In);

	string thingName = parser->current.str;
	Node *from = ParseExpression(parser, lexer, 0, arena);

	if (parser->current.kind == TokenKind_DotDotLess)
	{
		// for it in from..<to { statements; }

		// expands into:
		// {
		//     $to_copy := to;
		//     for it := from; it < $to_copy; it = it + 1
		//         { body }
		// }

		if (iterateByPointer)
		{
			ErrorAtCurrent(parser, "cannot iterate by pointer in a range-based for loop");
		}

		AdvanceToken(parser, lexer); // eat the '..<'

		Node *to = ParseExpression(parser, lexer, 0, arena);

		{
			// $to_copy := to;

			VarDeclNode *varDeclNode = MakeNode<VarDeclNode>(parser->current.location, arena);
			varDeclNode->name = "$to_copy";
			varDeclNode->expr = to;
			varDeclNode->type.kind = TypeKind_InferMe;

			array_add(&blockNode->statements, (Node *)varDeclNode);
		}

		ForNode *forNode = MakeNode<ForNode>(parser->current.location, arena);

		{
			// it := from;

			VarDeclNode *varDeclNode = MakeNode<VarDeclNode>(parser->current.location, arena);
			varDeclNode->name = iteratorName;
			varDeclNode->expr = from;
			varDeclNode->type.kind = TypeKind_InferMe;

			forNode->init = varDeclNode;
		}

		{
			// it < $to_copy

			VarNode *comparisonLhs = MakeNode<VarNode>(parser->current.location, arena);
			comparisonLhs->name = iteratorName;

			VarNode *comparisonRhs = MakeNode<VarNode>(parser->current.location, arena);
			comparisonRhs->name = "$to_copy";

			BinaryNode *comparisonNode = MakeNode<BinaryNode>(parser->current.location, arena);
			comparisonNode->op = BinaryOp_Less;
			comparisonNode->lhs = comparisonLhs;
			comparisonNode->rhs = comparisonRhs;

			forNode->cond = comparisonNode;
		}
	
		{
			// it = it + 1;

			VarNode *additionLhs = MakeNode<VarNode>(parser->current.location, arena);
			additionLhs->name = iteratorName;

			Int64LiteralNode *additionRhs = MakeNode<Int64LiteralNode>(parser->current.location, arena);
			additionRhs->value = 1;

			BinaryNode *assignRhs = MakeNode<BinaryNode>(parser->current.location, arena);
			assignRhs->op = BinaryOp_Add;
			assignRhs->rhs = additionLhs;
			assignRhs->lhs = additionRhs;

			VarNode *assignLhs = MakeNode<VarNode>(parser->current.location, arena);
			assignLhs->name = iteratorName;

			AssignNode *assignNode = MakeNode<AssignNode>(parser->current.location, arena);
			assignNode->lhs = assignLhs;
			assignNode->rhs = assignRhs;

			forNode->incr = assignNode;
		}

		forNode->body = ParseBlock(parser, lexer, arena);

		array_add(&blockNode->statements, (Node *)forNode);

		return blockNode;
	}
	else
	{
		Node *thing = from;

		// for it in thing { statements; }

		// expands into:
		// {
		//     $thing_copy := thing;
		//     for $it_index := 0; $it_index < $thing_copy.count; $it_index = $it_index + 1
		//     {
		//         it := $thing_copy[$it_index];
		//         { body }
		//     }
		// }

		{
			// $thing_copy := thing;

			VarDeclNode *varDeclNode = MakeNode<VarDeclNode>(parser->current.location, arena);
			varDeclNode->name = "$thing_copy";
			varDeclNode->expr = thing;
			varDeclNode->type.kind = TypeKind_InferMe;

			array_add(&blockNode->statements, (Node *)varDeclNode);
		}

		ForNode *forNode = MakeNode<ForNode>(parser->current.location, arena);

		{
			// $it_index := 0;

			Int64LiteralNode *zero = MakeNode<Int64LiteralNode>(parser->current.location, arena);
			zero->value = 0;

			VarDeclNode *varDeclNode = MakeNode<VarDeclNode>(parser->current.location, arena);
			varDeclNode->name = "$it_index";
			varDeclNode->expr = zero;
			varDeclNode->type.kind = TypeKind_InferMe;

			forNode->init = varDeclNode;
		}

		{
			// $it_index < $thing_copy.count

			FieldAccessNode *fieldAccessNode = MakeNode<FieldAccessNode>(parser->current.location, arena);
			fieldAccessNode->expr = MakeVarNode(parser->current.location, "$thing_copy", arena);
			fieldAccessNode->fieldName = "count";

			BinaryNode *comparisonNode = MakeNode<BinaryNode>(parser->current.location, arena);
			comparisonNode->op = BinaryOp_Less;
			comparisonNode->lhs = MakeVarNode(parser->current.location, "$it_index", arena);
			comparisonNode->rhs = fieldAccessNode;

			forNode->cond = comparisonNode;
		}

		{
			// $it_index = $it_index + 1;

			BinaryNode *assignRhs = MakeNode<BinaryNode>(parser->current.location, arena);
			assignRhs->op = BinaryOp_Add;
			assignRhs->rhs = MakeVarNode(parser->current.location, "$it_index", arena);
			assignRhs->lhs = MakeInt64Literal(parser->current.location, 1, arena);

			AssignNode *assignNode = MakeNode<AssignNode>(parser->current.location, arena);
			assignNode->lhs = MakeVarNode(parser->current.location, "$it_index", arena);
			assignNode->rhs = assignRhs;

			forNode->incr = assignNode;
		}

		BlockNode *loopBody = MakeNode<BlockNode>(parser->current.location, arena);
		loopBody->statements = PushBumpArray<Node *>(arena, 2);

		{
			// it := $thing_copy[$it_index];

			ArrayIndexAccessNode *arrayIndexAccess = MakeNode<ArrayIndexAccessNode>(parser->current.location, arena);
			arrayIndexAccess->arrayExpr = MakeVarNode(parser->current.location, "$thing_copy", arena);
			arrayIndexAccess->indexExpr = MakeVarNode(parser->current.location, "$it_index", arena);

			VarDeclNode *varDeclNode = MakeNode<VarDeclNode>(parser->current.location, arena);
			varDeclNode->name = iteratorName;
			varDeclNode->type.kind = TypeKind_InferMe;

			if (iterateByPointer)
			{
				AddressOfNode *addressOf = MakeNode<AddressOfNode>(parser->current.location, arena);
				addressOf->what = arrayIndexAccess;

				varDeclNode->expr = addressOf;
			}
			else
			{
				varDeclNode->expr = arrayIndexAccess;
			}

			array_add(&loopBody->statements, (Node *)varDeclNode);
		}

		{
			Node *actualLoopBody = ParseBlock(parser, lexer, arena);
			array_add(&loopBody->statements, actualLoopBody);
		}

		forNode->body = loopBody;

		array_add(&blockNode->statements, (Node *)forNode);

		return blockNode;
	}
}

internal Node *
ParseDeferStatement(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	DeferNode *node = MakeNode<DeferNode>(parser->current.location, arena);

	AdvanceToken(parser, lexer); // eat the 'defer'

	node->what = ParseBlockOrSingleStatement(parser, lexer, arena);

	return node;
}

internal Node *
ParseSwitchStatement(Parser *parser,
					 Lexer *lexer,
					 Arena *arena)
{
	SwitchNode *node = MakeNode<SwitchNode>(parser->current.location, arena);

	AdvanceToken(parser, lexer); // eat the 'switch'

	node->expr = ParseExpression(parser, lexer, 0, arena);

	ExpectToken(parser, lexer, TokenKind_OpenBrace);

	node->cases = PushBumpArray<CaseNode *>(arena, 64);

	while (!parser->hadError
		   && parser->current.kind != TokenKind_CloseBrace)
	{
		if (parser->current.kind == TokenKind_Case)
		{
			CaseNode *caseNode = MakeNode<CaseNode>(parser->current.location, arena);

			AdvanceToken(parser, lexer); // eat the 'case'

			caseNode->label = ParseExpression(parser, lexer, 0, arena);

			ExpectToken(parser, lexer, TokenKind_Colon);

			// TODO: parse multiple statements
			caseNode->body = ParseBlockOrSingleStatement(parser, lexer, arena);

			array_add(&node->cases, caseNode);
		}
		else if (parser->current.kind == TokenKind_Default)
		{
			if (!node->defaultBody)
			{
				AdvanceToken(parser, lexer); // eat the 'default'

				ExpectToken(parser, lexer, TokenKind_Colon);

				// TODO: parse multiple statements
				node->defaultBody = ParseBlockOrSingleStatement(parser, lexer, arena);
			}
			else
			{
				ErrorAtCurrent(parser, "default case was already defined");
			}
		}
		else
		{
			UnexpectedCurrentToken(parser);
		}
	}

	ExpectToken(parser, lexer, TokenKind_CloseBrace);

	return node;
}

internal Node *
ParsePrintStatement(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	// print expr;

	PrintNode *node = MakeNode<PrintNode>(parser->current.location, arena);

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

	VarDeclNode *node = MakeNode<VarDeclNode>(parser->current.location, arena);
	node->name = parser->current.str;

	// eat the variable name
	AdvanceToken(parser, lexer);

	ExpectToken(parser, lexer, TokenKind_Colon);

	node->type.kind = TypeKind_InferMe;

	if (parser->current.kind != TokenKind_Equal)
	{
		node->type = ParseType(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Equal)
	{
		// name : type = expr;
		// name : = expr;

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
	}
	else
	{
		UnexpectedCurrentToken(parser);
	}

	return node;
}

internal Node *
ParseAsmBlock(Parser *parser,
			  Lexer *lexer,
			  Arena *arena)
{
	AsmNode *node = MakeNode<AsmNode>(parser->current.location, arena);

	ExpectToken(parser, lexer, TokenKind_Asm);

	Token openBraceToken = parser->current;
	ExpectToken(parser, lexer, TokenKind_OpenBrace);

	while (!parser->hadError
		   && parser->current.kind != TokenKind_CloseBrace)
	{
		AdvanceToken(parser, lexer);
	}

	Token closeBraceToken = parser->current;
	ExpectToken(parser, lexer, TokenKind_CloseBrace);

	node->code.data = openBraceToken.str.data;
	node->code.count = closeBraceToken.str.data - openBraceToken.str.data;

	node->code.data++;
	node->code.count--;

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

	if (parser->current.kind == TokenKind_For)
	{
		Token peek0 = PeekToken(lexer, 1);
		Token peek1 = PeekToken(lexer, 2);

		if (peek1.kind == TokenKind_In
			|| peek0.kind == TokenKind_Star)
		{
			return ParseForeachStatement(parser, lexer, arena);
		}
		else
		{
			return ParseForStatement(parser, lexer, arena);
		}
	}

	if (parser->current.kind == TokenKind_Defer)
	{
		return ParseDeferStatement(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Switch)
	{
		return ParseSwitchStatement(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Print)
	{
		return ParsePrintStatement(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Yield)
	{
		// yield;
		YieldNode *node = MakeNode<YieldNode>(parser->current.location, arena);
		AdvanceToken(parser, lexer); // eat the 'yield'
		ExpectToken(parser, lexer, TokenKind_Semicolon);
		return node;
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

	if (parser->current.kind == TokenKind_Break)
	{
		BreakNode *node = MakeNode<BreakNode>(parser->current.location, arena);
		AdvanceToken(parser, lexer); // eat the 'break'
		ExpectToken(parser, lexer, TokenKind_Semicolon);
		return node;
	}

	if (parser->current.kind == TokenKind_Continue)
	{
		ContinueNode *node = MakeNode<ContinueNode>(parser->current.location, arena);
		AdvanceToken(parser, lexer); // eat the 'continue'
		ExpectToken(parser, lexer, TokenKind_Semicolon);
		return node;
	}

	if (parser->current.kind == TokenKind_Identifier)
	{
		Token peekToken = PeekToken(lexer);
		if (peekToken.kind == TokenKind_Colon)
		{
			return ParseVariableDeclaration(parser, lexer, arena);
		}
	}

	if (parser->current.kind == TokenKind_Asm)
	{
		return ParseAsmBlock(parser, lexer, arena);
	}

	if (parser->current.kind == TokenKind_Semicolon)
	{
		// bare semicolon - empty statement

		// call ExpectToken instead of AdvanceToken to handle numInsertSemicolons
		ExpectToken(parser, lexer, TokenKind_Semicolon);

		return nullptr;
	}

	Node *expr = ParseExpression(parser, lexer, 0, arena);
	if (parser->current.kind == TokenKind_Equal)
	{
		// assignment
		// lhs = rhs;

		AssignNode *node = MakeNode<AssignNode>(parser->current.location, arena);
		node->lhs = expr;

		AdvanceToken(parser, lexer); // eat the '='

		node->rhs = ParseExpression(parser, lexer, 0, arena);

		ExpectToken(parser, lexer, TokenKind_Semicolon);

		return node;
	}

	{
		// compound assignment operators

		struct CompoundOperator
		{
			TokenKind token;
			BinaryOp op;
		};

		CompoundOperator operators[] =
		{
			{TokenKind_PlusEqual,    BinaryOp_Add},
			{TokenKind_MinusEqual,   BinaryOp_Subtract},
			{TokenKind_PercentEqual, BinaryOp_Modulo},
			{TokenKind_StarEqual,    BinaryOp_Multiply},
			{TokenKind_SlashEqual,   BinaryOp_Multiply},
		};

		for (auto &it : operators)
		{
			if (parser->current.kind == it.token)
			{
				AssignNode *node = MakeNode<AssignNode>(parser->current.location, arena);
				node->lhs = expr;

				AdvanceToken(parser, lexer);

				Node *rhs = ParseExpression(parser, lexer, 0, arena);

				node->rhs = MakeBinaryNode(it.op, parser->current.location, expr, rhs, arena);

				ExpectToken(parser, lexer, TokenKind_Semicolon);

				return node;
			}
		}
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

	FuncNode *node = MakeNode<FuncNode>(parser->current.location, arena);
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

		ParamNode *param = MakeNode<ParamNode>(parser->current.location, arena);
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

	while (parser->current.kind == TokenKind_Hash)
	{
		AdvanceToken(parser, lexer); // eat the '#'

		if (parser->current.str == "foreign")
		{
			ExpectToken(parser, lexer, TokenKind_Identifier);

			if (parser->current.kind == TokenKind_String)
			{
				node->foreignLinkName = parser->current.str;

				Assert(node->foreignLinkName.count >= 2);
				node->foreignLinkName.data++;
				node->foreignLinkName.count -= 2;

				AdvanceToken(parser, lexer);
			}

			node->isForeign = true;
		}
		else if (parser->current.str == "coroutine")
		{
			ExpectToken(parser, lexer, TokenKind_Identifier);

			node->isCoroutine = true;
		}
		else if (parser->current.str == "variadic")
		{
			ExpectToken(parser, lexer, TokenKind_Identifier);

			node->isVariadic = true;
		}
		else
		{
			UnexpectedCurrentToken(parser);
		}
	}

	if (!node->isForeign)
	{
		node->body = ParseBlock(parser, lexer, arena);
	}
	else
	{
		ExpectToken(parser, lexer, TokenKind_Semicolon);
	}

	return node;
}

internal Node *
ParseStructDefinition(Parser *parser,
					  Lexer *lexer,
					  Arena *arena)
{
	const int MAX_STRUCT_FIELDS = 32;

	StructDeclNode *node = MakeNode<StructDeclNode>(parser->current.location, arena);
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
		StructFieldDeclNode *field = MakeNode<StructFieldDeclNode>(parser->current.location, arena);
		field->name = parser->current.str;

		ExpectToken(parser, lexer, TokenKind_Identifier);

		ExpectToken(parser, lexer, TokenKind_Colon);

		field->type = ParseType(parser, lexer, arena);

		ExpectToken(parser, lexer, TokenKind_Semicolon);

		array_add(&node->fields, field);
	}

	ExpectToken(parser, lexer, TokenKind_CloseBrace);

	return node;
}

internal Node *
ParseEnumDefinition(Parser *parser,
					Lexer *lexer,
					Arena *arena)
{
	const int MAX_ENUMERATORS = 32;

	EnumDeclNode *node = MakeNode<EnumDeclNode>(parser->current.location, arena);
	node->name = parser->current.str;
	node->enumerators = PushBumpArray<EnumeratorDeclNode *>(arena, MAX_ENUMERATORS);

	AdvanceToken(parser, lexer); // eat the enum name

	ExpectToken(parser, lexer, TokenKind_Colon);
	ExpectToken(parser, lexer, TokenKind_Colon);

	ExpectToken(parser, lexer, TokenKind_Enum);

	ExpectToken(parser, lexer, TokenKind_OpenBrace);

	while (!parser->hadError
		   && parser->current.kind != TokenKind_CloseBrace)
	{
		EnumeratorDeclNode *enumerator = MakeNode<EnumeratorDeclNode>(parser->current.location, arena);
		enumerator->name = parser->current.str;

		ExpectToken(parser, lexer, TokenKind_Identifier);

		ExpectToken(parser, lexer, TokenKind_Semicolon);

		array_add(&node->enumerators, enumerator);
	}

	ExpectToken(parser, lexer, TokenKind_CloseBrace);

	return node;
}

internal Node *
ParseConstantDefinition(Parser *parser,
						Lexer *lexer,
						Arena *arena)
{
	ConstantDeclNode *node = MakeNode<ConstantDeclNode>(parser->current.location, arena);
	node->name = parser->current.str;

	AdvanceToken(parser, lexer); // eat the constant name

	ExpectToken(parser, lexer, TokenKind_Colon);
	ExpectToken(parser, lexer, TokenKind_Colon);

	node->expr = ParseExpression(parser, lexer, 0, arena);

	ExpectToken(parser, lexer, TokenKind_Semicolon);

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

		if (peekToken0.kind == TokenKind_Colon)
		{
			if (peekToken1.kind == TokenKind_Colon)
			{
				if (peekToken2.kind == TokenKind_Proc)
				{
					return ParseFunctionDefinition(parser, lexer, arena);
				}

				if (peekToken2.kind == TokenKind_Struct)
				{
					return ParseStructDefinition(parser, lexer, arena);
				}

				if (peekToken2.kind == TokenKind_Enum)
				{
					return ParseEnumDefinition(parser, lexer, arena);
				}

				// compile-time constant
				// FOO :: 123;
				return ParseConstantDefinition(parser, lexer, arena);
			}

			// global variable declaration
			// foo: int;
			return ParseVariableDeclaration(parser, lexer, arena);
		}
	}

	if (parser->current.kind == TokenKind_Semicolon)
	{
		// bare semicolon - empty statement
		AdvanceToken(parser, lexer);
		return nullptr;
	}

	if (parser->current.kind == TokenKind_Asm)
	{
		return ParseAsmBlock(parser, lexer, arena);
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

	BlockNode *block = MakeNode<BlockNode>({}, arena);
	block->statements = PushBumpArray<Node *>(arena, MAX_STATEMENTS);

	while (!parser->hadError
		   && parser->current.kind != TokenKind_EOF)
	{
		Node *statement = ParseTopLevelStatement(parser, lexer, arena);
		if (parser->hadError)
		{
			break;
		}

		if (statement)
		{
			array_add(&block->statements, statement);
		}
		else
		{
			// it's an empty statement - ignore it
		}
	}

	return block;
}
