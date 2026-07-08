#pragma once

#include "common.h"

struct Constant
{
	string name;
	i64 value;
};

struct ConstantsTable
{
	StaticBumpArray<Constant, 256> constants;
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
	Constant *result = ArrayAdd(&table->constants, {});
	result->name = name;

	return result;
}
