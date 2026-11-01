#pragma once

#include "base_types.h"
#include "base_string.h"
#include "base_string_builder.h"
#include "base_assert.h"
#include "base_allocator.h"

enum FormatSpec : u32
{
	FormatSpec_v,
	FormatSpec_d,
	FormatSpec_s,
};

struct FormatContext
{
	string format;
	FormatSpec spec;
	string_builder *builder;
};

inline void
type_formatter(FormatContext *context, int value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_d);
	builder_write(context->builder, value);
}

inline void
type_formatter(FormatContext *context, string value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_s);
	builder_write(context->builder, value);
}

inline void
type_formatter(FormatContext *context, const char *value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_s);
	string str = {(char *)value, strlen(value)};
	builder_write(context->builder, str);
}

inline bool
write_until_placeholder(FormatContext *context)
{
	while (context->format)
	{
		char ch = context->format[0];

		if (ch == '%')
		{
			string_advance(&context->format);
			ch = context->format[0];

			if (ch == 'v')
			{
				context->spec = FormatSpec_v;
			}
			else if (ch == 'd')
			{
				context->spec = FormatSpec_d;
			}
			else if (ch == 's')
			{
				context->spec = FormatSpec_s;
			}
			else
			{
				Assert(false, "invalid print format specifier");
			}

			string_advance(&context->format);

			return true;
		}
		else
		{
			builder_write(context->builder, ch);
			string_advance(&context->format);
		}
	}

	return false;
}

inline void
string_format_impl(FormatContext *context)
{
	while (write_until_placeholder(context))
	{
		Assert(false, "more placeholders than arguments");
	}
}

template <typename T, typename... Rest>
inline void
string_format_impl(FormatContext *context,
				   T first, Rest... rest)
{
	bool found = write_until_placeholder(context);
	if (!found)
	{
		Assert(false, "more arguments than placeholders");
	}

	type_formatter(context, first);

	string_format_impl(context, rest...);
}

template <typename... Args>
inline string
string_format(Allocator allocator,
			  string format,
			  Args... args)
{
	string_builder builder = {};
	builder_init(&builder, allocator);

	FormatContext context = {};
	context.format = format;
	context.builder = &builder;

	string_format_impl(&context, args...);

	return builder_to_string(*context.builder);
}

template <typename... Args>
inline void
builder_print(string_builder *builder,
			  string format,
			  Args... args)
{
	FormatContext context = {};
	context.format = format;
	context.builder = builder;

	string_format_impl(&context, args...);
}
