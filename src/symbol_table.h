#pragma once

#include "base/base.h"
#include "type.h"

struct Symbol
{
	Type type;
	string name;
	int stackOffset;
};

struct SymbolTable
{
	dynamic_array<Symbol> symbols;
	usize scopeStart;
	int stackSize;
	int maxStackSize;
};

inline Symbol *
LookupSymbol(SymbolTable *table,
			 string name,
			 usize scopeStart)
{
	// search backwards
	for (usize i = table->symbols.count;
		 i-- != scopeStart;)
	{
		if (table->symbols[i].name == name)
		{
			return &table->symbols[i];
		}
	}

	return nullptr;
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

	Symbol *symbol = array_add(&table->symbols, {});
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

inline void
ClearTable(SymbolTable *table)
{
	table->symbols.count = 0;
	table->scopeStart = 0;
	table->stackSize = 0;
	table->maxStackSize = 0;
}
