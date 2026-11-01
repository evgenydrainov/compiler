#pragma once

#include "base_types.h"
#include "base_string.h"
#include "base_allocator.h"
#include "base_dynamic_array.h"
#include <stdio.h> // for snprintf

struct string_builder
{
	dynamic_array<char> buffer;
};

inline void
builder_init(string_builder *builder, Allocator allocator)
{
	builder->buffer.allocator = allocator;
}

inline void
builder_write(string_builder *builder, char ch)
{
	array_add(&builder->buffer, ch);
}

inline void
builder_write(string_builder *builder, string str)
{
	array_add_many(&builder->buffer, str);
}

inline void
builder_write(string_builder *builder, int value)
{
#if 1
	if (value == 0)
	{
		array_add(&builder->buffer, '0');
		return;
	}

	if (value < 0)
	{
		array_add(&builder->buffer, '-');
		value = -value;
	}

	int magnitude = 1;
	for (int v = value/10; v > 0; v /= 10)
	{
		magnitude *= 10;
	}

	while (magnitude > 0)
	{
		char ch = '0' + ((value / magnitude) % 10);
		array_add(&builder->buffer, ch);
		magnitude /= 10;
	}
#else
	int length = snprintf(nullptr, 0, "%d", value) + 1;

	array_reserve(&builder->buffer, builder->buffer.count + length);

	snprintf(builder->buffer.end(), length, "%d", value);

	builder->buffer.count += length;

	// pop the null terminator
	builder->buffer.count--;
#endif
}

inline string
builder_to_string(string_builder builder)
{
	return {builder.buffer.data, builder.buffer.count};
}
