#pragma once

#include "base/base.h"

struct MacroDeclNode;

struct Macro
{
	string name;
	MacroDeclNode *decl;
};

struct MacroTable
{
	dynamic_array<Macro> macros;
};

inline Macro *
LookupMacro(MacroTable *table,
			string name)
{
	foreach (it, table->macros)
	{
		if (it->name == name)
		{
			return it;
		}
	}

	return nullptr;
}

inline Macro *
DeclareMacro(MacroTable *table, string name)
{
	Macro *result = array_add(&table->macros, {});
	result->name = name;

	return result;
}
