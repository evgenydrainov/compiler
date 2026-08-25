#pragma once

#include "base/base.h"

struct Constant
{
	string name;
	i64 value;
};

struct ConstantsTable
{
	static_bump_array<Constant, 256> constants;
};

inline Constant *
LookupConstant(ConstantsTable *table,
			   string name)
{
	Constant *result = nullptr;

	for (usize i = 0;
		 i < table->constants.count;
		 i++)
	{
		Constant *constant = &table->constants[i];
		if (constant->name == name)
		{
			result = constant;
			break;
		}
	}

	return result;
}

inline Constant *
DeclareConstant(ConstantsTable *table,
				string name)
{
	Constant *result = array_add(&table->constants, {});
	result->name = name;

	return result;
}
