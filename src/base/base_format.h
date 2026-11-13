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
	FormatSpec_u,
	FormatSpec_s,
	FormatSpec_f,
	FormatSpec_X,
	FormatSpec_zu,
	FormatSpec_lld,
	FormatSpec_llu,
	FormatSpec_llX,
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
type_formatter(FormatContext *context, u32 value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_u
		   || context->spec == FormatSpec_X
		   || context->spec == FormatSpec_zu);
	if (context->spec == FormatSpec_X)
	{
		builder_write_hex(context->builder, value);
	}
	else
	{
		builder_write(context->builder, value);
	}
}

inline void
type_formatter(FormatContext *context, i64 value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_lld);
	builder_write(context->builder, value);
}

inline void
type_formatter(FormatContext *context, u64 value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_llu
		   || context->spec == FormatSpec_llX
		   || context->spec == FormatSpec_zu);
	if (context->spec == FormatSpec_llX)
	{
		builder_write_hex(context->builder, value);
	}
	else
	{
		builder_write(context->builder, value);
	}
}

inline void
type_formatter(FormatContext *context, f32 value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_f);
	builder_write(context->builder, value);
}

inline void
type_formatter(FormatContext *context, f64 value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_f);
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
type_formatter(FormatContext *context, bool value)
{
	Assert(context->spec == FormatSpec_v);
	builder_write(context->builder, value);
}

inline void
type_formatter(FormatContext *context, const char *value)
{
	Assert(context->spec == FormatSpec_v
		   || context->spec == FormatSpec_s);
	builder_write(context->builder, value);
}

inline bool
write_until_placeholder(FormatContext *context)
{
	while (context->format)
	{
		if (context->format[0] != '%')
		{
			builder_write(context->builder, context->format[0]);
			string_advance(&context->format);
			continue;
		}

		string_advance(&context->format);

		if (!context->format)
		{
			Assert(false, "incomplete print format specifier");
			return false;
		}

		if (context->format[0] == '%')
		{
			builder_write(context->builder, '%');
			string_advance(&context->format);
			continue;
		}
			
		usize length = 1;

		if (context->format[0] == 'v')
		{
			context->spec = FormatSpec_v;
		}
		else if (context->format[0] == 'd')
		{
			context->spec = FormatSpec_d;
		}
		else if (context->format[0] == 'u')
		{
			context->spec = FormatSpec_u;
		}
		else if (context->format[0] == 's')
		{
			context->spec = FormatSpec_s;
		}
		else if (context->format[0] == 'f')
		{
			context->spec = FormatSpec_f;
		}
		else if (context->format[0] == 'X')
		{
			context->spec = FormatSpec_X;
		}
		else if (starts_with(context->format, "zu"))
		{
			context->spec = FormatSpec_zu;
			length = 2;
		}
		else if (starts_with(context->format, "lld"))
		{
			context->spec = FormatSpec_lld;
			length = 3;
		}
		else if (starts_with(context->format, "llu"))
		{
			context->spec = FormatSpec_llu;
			length = 3;
		}
		else if (starts_with(context->format, "llX"))
		{
			context->spec = FormatSpec_llX;
			length = 3;
		}
		else
		{
			Assert(false, "invalid print format specifier");
		}

		string_advance(&context->format, length);

		return true;
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

// TODO: you can't mem_free(str.data, str.count) because the count is wrong and
// you have to do array_free(&builder.buffer)
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

//template <typename... Args>
//inline void
//builder_print(string_builder *builder,
//			  string format,
//			  Args... args)
//{
//	FormatContext context = {};
//	context.format = format;
//	context.builder = builder;
//
//	string_format_impl(&context, args...);
//}
