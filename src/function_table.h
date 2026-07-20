#pragma once

#include "common.h"
#include "type.h"

struct Parameter
{
	Type type;
};

struct Function
{
	string name;

	StaticBumpArray<Parameter, 32> params;

	Type returnType;

	string linkName;
};

struct FunctionTable
{
	Function functions[128];
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
