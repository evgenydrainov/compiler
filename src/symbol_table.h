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
};

inline Symbol *
LookupSymbol(SymbolTable *table, string name)
{
	Symbol *result = nullptr;

	// search backwards
	for (int i = table->count;
		 i--;)
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

	Assert(table->count < ArrayCount(table->symbols));

	Symbol *symbol = &table->symbols[table->count++];
	*symbol = {};
	symbol->name = name;
	symbol->offset = table->stackSize;

	return symbol;
}
