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
builder_write(string_builder *builder, u64 value)
{
	if (value == 0)
	{
		array_add(&builder->buffer, '0');
		return;
	}

	u64 magnitude = 1;
	for (u64 v = value/10; v > 0; v /= 10)
	{
		magnitude *= 10;
	}

	while (magnitude > 0)
	{
		char ch = '0' + ((value / magnitude) % 10);
		array_add(&builder->buffer, ch);
		magnitude /= 10;
	}
}

inline void
builder_write(string_builder *builder, i64 value)
{
	if (value < 0)
	{
		array_add(&builder->buffer, '-');
		value = -value;
	}

	builder_write(builder, (u64)value);
}

inline void
builder_write(string_builder *builder, u32 value)
{
	builder_write(builder, (u64)value);
}

inline void
builder_write(string_builder *builder, i32 value)
{
	builder_write(builder, (i64)value);
}

inline void
builder_write_hex(string_builder *builder, u64 value)
{
	if (value == 0)
	{
		array_add(&builder->buffer, '0');
		return;
	}

	u64 magnitude = 1;
	for (u64 v = value/16; v > 0; v /= 16)
	{
		magnitude *= 16;
	}

	local_persist char alphabet[16] =
	{
		'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
	};

	while (magnitude > 0)
	{
		char ch = alphabet[(value / magnitude) % 16];
		array_add(&builder->buffer, ch);
		magnitude /= 16;
	}
}

inline void
builder_write_hex(string_builder *builder, u32 value)
{
	builder_write_hex(builder, (u64)value);
}

inline void
builder_write(string_builder *builder, f64 value)
{
	int length = snprintf(nullptr, 0, "%f", value) + 1;

	array_reserve(&builder->buffer, builder->buffer.count + length);

	snprintf(builder->buffer.end(), length, "%f", value);

	builder->buffer.count += length;

	// pop the null terminator
	builder->buffer.count--;
}

inline void
builder_write(string_builder *builder, f32 value)
{
	builder_write(builder, (f64)value);
}

inline string
builder_to_string(string_builder builder)
{
	return {builder.buffer.data, builder.buffer.count};
}
