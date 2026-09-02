#pragma once

#include "base/base.h"

struct Node;

struct PrintContext
{
	int indentation;
	bool fromNewLine;
	int suppressNewLines;
};

void PrintProgram(PrintContext *context,
				  Node *_program);
