#pragma once

#include "base_types.h"
#include "base_arena.h"
#include "base_string.h"
#include <stdarg.h>

struct string_builder
{
	Arena *arena;
	char *data;
	usize count;
};

inline void
builder_init(string_builder *builder, Arena *arena)
{
	builder->arena = arena;
}

inline void
builder_write(string_builder *builder, string str)
{
	builder->data = (char *)grow_allocation(builder->arena, builder->data, builder->count, builder->count + str.count);
	for (char ch : str)
	{
		builder->data[builder->count++] = ch;
	}
}

inline void
builder_write_fmt(string_builder *builder, char *format, ...)
{
	va_list args;
	va_start(args, format);

	string str = vtprintf(format, args);

	va_end(args);

	builder_write(builder, str);
}

inline string
builder_to_string(string_builder builder)
{
	string result = {};
	result.data = builder.data;
	result.count = builder.count;

	return result;
}
