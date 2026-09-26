#pragma once

#include "base/base.h"

struct Constant
{
	string name;
	i64 value;
};

struct ConstantsTable
{
	dynamic_array<Constant> constants;
};

inline Constant *
LookupConstant(ConstantsTable *table,
			   string name)
{
	foreach (it, table->constants)
	{
		if (it->name == name)
		{
			return it;
		}
	}

	return nullptr;
}

inline Constant *
DeclareConstant(ConstantsTable *table,
				string name)
{
	Constant *result = array_add(&table->constants, {});
	result->name = name;

	return result;
}
