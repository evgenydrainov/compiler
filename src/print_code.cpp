#if 0
struct PrintContext
{
	int indentation;
};

internal void
Print(PrintContext *context,
	  char *format, ...)
{
	for (int i = 0; i < context->indentation; i++)
	{
		printf("    ");
	}

	va_list args;
	va_start(args, format);

	vprintf(format, args);

	va_end(args);
}

internal void
PrintCode(PrintContext *context,
		  Node *baseNode)
{
	if (!baseNode)
	{
		return;
	}

	switch (baseNode->kind)
	{
		case NodeKind_Block:
		{
			BlockNode *node = As<BlockNode>(baseNode);

			Print(context, "{\n");

			context->indentation++;

			for (Node *statement : node->statements)
			{
				PrintCode(context, statement);
			}

			context->indentation--;

			Print(context, "}\n");
		} break;

		case NodeKind_Func:
		{
			FuncNode *node = As<FuncNode>(baseNode);

			Print(context, STR_FMT " :: proc(", STR_ARG(node->name));

			for (int i = 0; i < node->numParams; i++)
			{
				ParamNode *param = As<ParamNode>(node->params[i]);
				Print(context, STR_FMT ", ", STR_ARG(param->name));
			}

			Print(context, ")\n");
			
			PrintCode(context, node->body);

			Print(context, "\n");
		} break;

		default: {} break;
	}
}
#endif
