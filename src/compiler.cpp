#include "compiler.h"

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic_pass.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <io.h>
#include <process.h>

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

struct FindLinkerResult
{
	char *linkExePath;
	char *libraryPath;
};

internal FindLinkerResult
FindLinkerPath()
{
	char *versionsToTry[] =
	{
		"14.44.35207",
		"14.42.34433",
	};

	for (char *version : versionsToTry)
	{
		char linkExePath[1024];
		sprintf_s(linkExePath,
				  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\%s\\bin\\Hostx64\\x64\\link.exe",
				  version);

		char libraryPath[1024];
		sprintf_s(libraryPath,
				  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\%s\\lib\\x64",
				  version);

		if (_access(linkExePath, 0) == 0)
		{
			FindLinkerResult result = {};

			result.linkExePath = _strdup(linkExePath);
			result.libraryPath = _strdup(libraryPath);

			return result;
		}
	}

	fprintf(stderr, "could not find link.exe");
	exit(1);
}

CompileResult
Compile(CompileOptions *options)
{
	string compilerExeFileDir = get_executable_dir();
	string modulesDir = string_concat(compilerExeFileDir, "modules/");

	LexerContext lexerContext = {};
	lexerContext.compilerExeFileDir = compilerExeFileDir;
	lexerContext.modulesDir = modulesDir;

	Lexer lexer = {};
	lexer.context = &lexerContext;

	{
		string builtinFilePath = string_concat(compilerExeFileDir, "modules/builtin.c");

		string builtinFileSrc = read_entire_file(builtinFilePath);
		if (builtinFileSrc.count == 0)
		{
			fprintf(stderr, "cannot open file " STR_FMT_QUOTED " for reading\n", STR_ARG(builtinFilePath));
			return CompileResult_CannotReadFile;
		}

		lexer.line = 1;
		lexer.current = builtinFileSrc.data;
		lexer.fileName = builtinFilePath;
		lexer.lineStart = lexer.current;
	}

	{
		string sourceCode = read_entire_file(options->inputFilePath);
		if (sourceCode.count == 0)
		{
			fprintf(stderr, "cannot open file " STR_FMT_QUOTED " for reading\n", STR_ARG(options->inputFilePath));
			return CompileResult_CannotReadFile;
		}

		LexerFrame frame = {};
		frame.current = sourceCode.data;
		frame.line = 1;
		frame.fileName = options->inputFilePath;
		frame.lineStart = frame.current;

		array_add(&lexer.includeStack, frame);
	}

	Arena arena = {};
	arena.capacity = Megabytes(4);
	arena.data = (u8 *)malloc(arena.capacity);

	defer { free(arena.data); };

	Parser parser = {};
	parser.current = GetToken(&lexer);

	Node *program = ParseProgram(&parser, &lexer, &arena);

	if (!program
		|| parser.hadError)
	{
		return CompileResult_ParseError;
	}

	SemanticContext semanticContext = {};
	semanticContext.arenaForAst = &arena;
	SemanticPass(program, &semanticContext, &arena);

	if (semanticContext.hadError)
	{
		return CompileResult_SemanticError;
	}

	//{
	//	PrintContext context = {};
	//	PrintCode(&context, program);
	//}

	CodegenContext codegenContext = {};
	codegenContext.funcTable = semanticContext.funcTable;
	codegenContext.cstringLiterals = semanticContext.cstringLiterals;
	codegenContext.stringLiterals = semanticContext.stringLiterals;
	codegenContext.deferStack = PushBumpArray<Node *>(&arena, 256);

	string asmFilePath = string_concat(options->outputFilePath, ".asm");
	string objFilePath = string_concat(options->outputFilePath, ".obj");

	char *asmFilePathCStr = to_cstring(asmFilePath);
	char *objFilePathCStr = to_cstring(objFilePath);

	{
		FILE *out;
		fopen_s(&out, asmFilePathCStr, "wb");

		if (!out)
		{
			fprintf(stderr, "cannot open file '%s' for writing\n", asmFilePathCStr);
			return CompileResult_CannotWriteFile;
		}

		codegenContext.out = out;
		Generate_x86_64(program, &codegenContext);

		fclose(out);
	}

	{
		if (_spawnl(_P_WAIT,
					"C:\\Users\\Username\\AppData\\Local\\bin\\NASM\\nasm.exe",
					"nasm",
					"-g",
					"-f", "win64",
					"-o", objFilePathCStr,
					asmFilePathCStr,
					nullptr) != 0)
		{
			return CompileResult_NasmError;
		}
	}

	{
		FindLinkerResult findResult = FindLinkerPath();

		char libraryArg[1024];
		sprintf_s(libraryArg, "/LIBPATH:\"%s\"", findResult.libraryPath);

		if (_spawnl(_P_WAIT,
					findResult.linkExePath,
					"link",
					"/nologo", "/DEBUG", "/INCREMENTAL:NO", "/OPT:REF", "/OPT:ICF",
					"/SUBSYSTEM:CONSOLE",
					objFilePathCStr,
					"raylib_x86-64_vs2022_mt.lib",
					"libcmt.lib",
					"legacy_stdio_definitions.lib",
					"kernel32.lib", "user32.lib", "gdi32.lib", "shell32.lib", "winmm.lib",
					libraryArg,
					"/LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\ucrt\\x64\"",
					"/LIBPATH:\"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\um\\x64\"",
					"/LIBPATH:\"C:\\Users\\Username\\source\\repos\\compiler\\modules\\raylib\"",
					nullptr) != 0)
		{
			return CompileResult_LinkerError;
		}
	}

	// printf("Arena usage: %.2f%%\n", 100.0f*(arena.pos/(float)arena.capacity));

	return CompileResult_Success;
}
