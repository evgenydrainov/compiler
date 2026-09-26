#pragma once

#include "base/base.h"
#include "type.h"

struct TypeTable
{
	dynamic_array<Type> types;
};

inline Type *
LookupType(TypeTable *table,
		   string name)
{
	foreach (it, table->types)
	{
		if (it->name == name)
		{
			return it;
		}
	}

	return nullptr;
}

inline Type *
DeclareType(TypeTable *table, string name)
{
	Type *type = array_add(&table->types, {});
	type->name = name;

	return type;
}
