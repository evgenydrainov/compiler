#pragma once

#include "common.h"
#include "type.h"

struct TypeTable
{
	Type types[256];
	int count;
};

inline Type *
LookupType(TypeTable *table,
		   string name)
{
	Type *result = nullptr;

	for (int i = 0;
		 i < table->count;
		 i++)
	{
		Type *type = &table->types[i];
		if (type->name == name)
		{
			result = type;
			break;
		}
	}

	return result;
}

inline Type *
DeclareType(TypeTable *table, string name)
{
	Assert(table->count < ArrayCount(table->types));

	Type *type = &table->types[table->count++];
	*type = {};
	type->name = name;

	return type;
}
