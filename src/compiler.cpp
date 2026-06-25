#include "compiler.h"

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic_pass.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <io.h>
#include <process.h>

#if 0
internal void
PrintTree(Node *baseNode,
		  const char *prefix,
		  bool isLeft)
{
	if (!baseNode)
	{
		return;
	}

	{
		const char *name = GetNodeKindName(baseNode->kind);
		if (baseNode->kind == NodeKind_Binary)
		{
			name = GetBinaryOpName(As<BinaryNode>(baseNode)->op);
		}

		printf("%s%s%s", prefix, isLeft ? "+-- " : "\\-- ", name);
	}

	printf(" (line=%d)", baseNode->line);

	if (baseNode->kind == NodeKind_Number)
	{
		NumberNode *n = As<NumberNode>(baseNode);
		printf(" (%lld)", n->int64Value);
	}
	else if (baseNode->kind == NodeKind_VarDecl)
	{
		VarDeclNode *n = As<VarDeclNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(n->name));
	}
	else if (baseNode->kind == NodeKind_Var)
	{
		VarNode *n = As<VarNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(n->name));
	}
	else if (baseNode->kind == NodeKind_Func)
	{
		FuncNode *n = As<FuncNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(n->name));
	}
	else if (baseNode->kind == NodeKind_Call)
	{
		CallNode *n = As<CallNode>(baseNode);
		printf(" (" STR_FMT ")", STR_ARG(n->name));
	}

	printf("\n");

	char childPrefix[256];
	snprintf(childPrefix, sizeof(childPrefix), "%s%s", prefix, isLeft ? "|   " : "    ");

	switch (baseNode->kind)
	{
		case NodeKind_Block:
		{
			BlockNode *n = As<BlockNode>(baseNode);
			for (int i = 0;
				 i < n->statements.count;
				 i++)
			{
				Node *statement = n->statements[i];
				PrintTree(statement, childPrefix, i != n->statements.count-1);
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

		default: {} break;
	}
}
#endif

internal string
LoadFile(const char *fileName)
{
	string result = {};
	
	FILE *file;
	fopen_s(&file, fileName, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		usize fileSize = ftell(file);

		char *fileData = (char *)malloc(fileSize + 1);
		if (fileData)
		{
			fseek(file, 0, SEEK_SET);
			if (fread(fileData, 1, fileSize, file) == fileSize)
			{
				fileData[fileSize] = 0;
				result = {fileData, fileSize};
			}
		}
		
		fclose(file);
	}

	return result;
}

struct FindLinkerResult
{
	const char *linkExePath;
	const char *libraryPath;
};

internal FindLinkerResult
FindLinkerPath()
{
	const char *versionsToTry[] =
	{
		"14.44.35207",
		"14.42.34433",
	};

	for (const char *version : versionsToTry)
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
Compile(CompileOptions options)
{
	string sourceCode = LoadFile(options.inputFilePath);
	if (sourceCode.count == 0)
	{
		fprintf(stderr, "cannot open file '%s' for reading\n", options.inputFilePath);
		return CompileResult_CannotReadFile;
	}

	defer { free(sourceCode.data); };

	Lexer lexer = {};
	lexer.line = 1;
	lexer.current = sourceCode.data;

	Arena arena = {};
	arena.capacity = Megabytes(1);
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
	SemanticPass(program, &semanticContext, &arena);

	if (semanticContext.hadError)
	{
		return CompileResult_SemanticError;
	}

	// PrintTree(program, "", false);

	CodegenContext codegenContext = {};

	{
		char fileName[1024];
		sprintf_s(fileName, "%s.asm", options.outputFilePath);

		FILE *out;
		fopen_s(&out, fileName, "wb");
		if (!out)
		{
			fprintf(stderr, "cannot open file '%s' for writing\n", fileName);
			return CompileResult_CannotWriteFile;
		}

		Generate_x86_64(program, out, &codegenContext);
		fclose(out);
	}

	{
		char outputPath[1024];
		sprintf_s(outputPath, "%s.obj", options.outputFilePath);

		char inputPath[1024];
		sprintf_s(inputPath, "%s.asm", options.outputFilePath);

		if (_spawnl(_P_WAIT,
					"C:\\Users\\Username\\AppData\\Local\\bin\\NASM\\nasm.exe",
					"nasm",
					"-f", "win64",
					"-o", outputPath,
					inputPath,
					nullptr) != 0)
		{
			return CompileResult_NasmError;
		}
	}

	{
		FindLinkerResult findResult = FindLinkerPath();

		char inputPath[1024];
		sprintf_s(inputPath, "%s.obj", options.outputFilePath);

		char libraryArg[1024];
		sprintf_s(libraryArg, "/LIBPATH:\"%s\"", findResult.libraryPath);

		if (_spawnl(_P_WAIT,
					findResult.linkExePath,
					"link",
					"/nologo",
					inputPath,
					"raylib.lib",
					"msvcrt.lib",
					"legacy_stdio_definitions.lib",
					"user32.lib", "gdi32.lib", "shell32.lib", "winmm.lib",
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
