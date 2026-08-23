#pragma once

#include "common.h"

struct MacroDeclNode;

struct Macro
{
	string name;
	MacroDeclNode *decl;
};

struct MacroTable
{
	StaticBumpArray<Macro, 128> macros;
};

inline Macro *
LookupMacro(MacroTable *table,
			string name)
{
	Macro *result = nullptr;

	for (usize i = 0;
		 i < table->macros.count;
		 i++)
	{
		Macro *macro = &table->macros[i];
		if (macro->name == name)
		{
			result = macro;
			break;
		}
	}

	return result;
}

inline Macro *
DeclareMacro(MacroTable *table, string name)
{
	Macro *result = array_add(&table->macros, {});
	result->name = name;

	return result;
}
