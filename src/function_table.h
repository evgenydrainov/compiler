#pragma once

#include "base/base.h"
#include "type.h"

struct Function
{
	string name;
	string linkName;

	ProcInfo *info;
};

struct FunctionTable
{
	dynamic_array<Function> functions;
};

inline Function *
LookupFunction(FunctionTable *table,
			   string name)
{
	foreach (it, table->functions)
	{
		if (it->name == name)
		{
			return it;
		}
	}

	return nullptr;
}

inline Function *
DeclareFunction(FunctionTable *table, string name)
{
	Function *function = array_add(&table->functions, {});
	function->name = name;

	return function;
}
