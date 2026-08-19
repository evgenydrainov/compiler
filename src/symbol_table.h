#pragma once

#include "common.h"
#include "type.h"

struct Symbol
{
	string name;
	int stackOffset;
	Type type;
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
DeclareSymbol(SymbolTable *table,
			  string name,
			  Type type)
{
	int size = SizeOfType(type);
	size = (int)align_forward(size, 8); // align to 8 for now

	table->stackSize += size;

	table->maxStackSize = Max(table->maxStackSize, table->stackSize);

	Assert(table->count < ArrayCount(table->symbols));

	Symbol *symbol = &table->symbols[table->count++];
	*symbol = {};
	symbol->name = name;
	symbol->stackOffset = table->stackSize;
	symbol->type = type;

	return symbol;
}

inline int
ReserveSpace(SymbolTable *table,
			 Type type)
{
	int size = SizeOfType(type);
	size = (int)align_forward(size, 8); // align to 8 for now

	table->stackSize += size;

	table->maxStackSize = Max(table->maxStackSize, table->stackSize);

	int stackOffset = table->stackSize;
	return stackOffset;
}
