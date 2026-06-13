#pragma once

#include "common.h"

struct Symbol
{
	string name;
	int offset;
};

struct SymbolTable
{
	Symbol symbols[256];
	int count;
	int stackSize;
	int maxStackSize;
	int scopeStart;
};

inline Symbol *
LookupSymbol(SymbolTable *table,
			 string name,
			 int scopeStart)
{
	Symbol *result = nullptr;

	// search backwards
	for (int i = table->count;
		 i-- != scopeStart;)
	{
		Symbol *symbol = &table->symbols[i];
		if (symbol->name == name)
		{
			result = symbol;
			break;
		}
	}

	return result;
}

inline Symbol *
DeclareSymbol(SymbolTable *table, string name)
{
	table->stackSize += 8;

	table->maxStackSize = Max(table->maxStackSize, table->stackSize);

	Assert(table->count < ArrayCount(table->symbols));

	Symbol *symbol = &table->symbols[table->count++];
	*symbol = {};
	symbol->name = name;
	symbol->offset = table->stackSize;

	return symbol;
}
