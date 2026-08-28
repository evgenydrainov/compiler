#if 0
internal void
PrintTree(Node *baseNode,
		  char *prefix,
		  bool isLeft)
{
	if (!baseNode)
	{
		return;
	}

	{
		char *name = GetNodeKindName(baseNode->kind);
		if (baseNode->kind == NodeKind_Binary)
		{
			name = GetBinaryOpName(As<BinaryNode>(baseNode)->op);
		}

		printf("%s%s%s", prefix, isLeft ? "+-- " : "\\-- ", name);
	}

	printf(" (line=%d)", baseNode->location.line);

	if (baseNode->kind == NodeKind_Int64Literal)
	{
		Int64LiteralNode *node = As<Int64LiteralNode>(baseNode);
		printf(" (%lld)", node->value);
	}
	else if (baseNode->kind == NodeKind_VarDecl)
	{
		VarDeclNode *node = As<VarDeclNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(node->name));
	}
	else if (baseNode->kind == NodeKind_Var)
	{
		VarNode *node = As<VarNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(node->name));
	}
	else if (baseNode->kind == NodeKind_Func)
	{
		FuncNode *node = As<FuncNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(node->name));
	}
	else if (baseNode->kind == NodeKind_Call)
	{
		CallNode *node = As<CallNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(node->name));
	}
	else if (baseNode->kind == NodeKind_StructDecl)
	{
		StructDeclNode *node = As<StructDeclNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(node->name));
	}
	else if (baseNode->kind == NodeKind_ConstantDecl)
	{
		ConstantDeclNode *node = As<ConstantDeclNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(node->name));
	}

	printf("\n");

	char childPrefix[256];
	snprintf(childPrefix, sizeof(childPrefix), "%s%s", prefix, isLeft ? "|   " : "    ");

	switch (baseNode->kind)
	{
		case NodeKind_Block:
		{
			BlockNode *node = As<BlockNode>(baseNode);
			for (int i = 0;
				 i < node->statements.count;
				 i++)
			{
				Node *statement = node->statements[i];
				PrintTree(statement, childPrefix, i != node->statements.count-1);
			}
		} break;

		case NodeKind_If:
		{
			IfNode *node = As<IfNode>(baseNode);
			if (node->elseBlock)
			{
				PrintTree(node->condition, childPrefix, true);
				PrintTree(node->thenBlock, childPrefix, true);
				PrintTree(node->elseBlock, childPrefix, false);
			}
			else
			{
				PrintTree(node->condition, childPrefix, true);
				PrintTree(node->thenBlock, childPrefix, false);
			}
		} break;

		case NodeKind_While:
		{
			WhileNode *node = As<WhileNode>(baseNode);
			PrintTree(node->condition, childPrefix, true);
			PrintTree(node->body, childPrefix, false);
		} break;

		case NodeKind_Binary:
		{
			BinaryNode *node = As<BinaryNode>(baseNode);
			if (node->lhs && node->rhs)
			{
				PrintTree(node->lhs, childPrefix, true);
				PrintTree(node->rhs, childPrefix, false);
			}
			else if (node->lhs)
			{
				PrintTree(node->lhs, childPrefix, false);
			}
			else if (node->rhs)
			{
				PrintTree(node->rhs, childPrefix, false);
			}
		} break;

		case NodeKind_Print:
		{
			PrintNode *node = As<PrintNode>(baseNode);
			PrintTree(node->expr, childPrefix, false);
		} break;

		case NodeKind_VarDecl:
		{
			VarDeclNode *node = As<VarDeclNode>(baseNode);
			PrintTree(node->expr, childPrefix, false);
		} break;

		case NodeKind_Assign:
		{
			AssignNode *node = As<AssignNode>(baseNode);
			PrintTree(node->lhs, childPrefix, true);
			PrintTree(node->rhs, childPrefix, false);
		} break;

		case NodeKind_Func:
		{
			FuncNode *node = As<FuncNode>(baseNode);
			PrintTree(node->body, childPrefix, false);
		} break;

		case NodeKind_Return:
		{
			ReturnNode *node = As<ReturnNode>(baseNode);
			PrintTree(node->expr, childPrefix, false);
		} break;

		case NodeKind_For:
		{
			ForNode *node = As<ForNode>(baseNode);
			PrintTree(node->init, childPrefix, true);
			PrintTree(node->cond, childPrefix, true);
			PrintTree(node->incr, childPrefix, true);
			PrintTree(node->body, childPrefix, false);
		} break;

		default: {} break;
	}
}
#endif
