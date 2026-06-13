#pragma once

#include "common.h"

struct Function
{
	string name;
};

struct FunctionTable
{
	Function functions[256];
	int count;
};

inline Function *
LookupFunction(FunctionTable *table,
			   string name)
{
	Function *result = nullptr;

	for (int i = 0;
		 i < table->count;
		 i++)
	{
		Function *function = &table->functions[i];
		if (function->name == name)
		{
			result = function;
			break;
		}
	}

	return result;
}

inline Function *
DeclareFunction(FunctionTable *table, string name)
{
	Assert(table->count < ArrayCount(table->functions));

	Function *function = &table->functions[table->count++];
	*function = {};
	function->name = name;

	return function;
}
