#pragma once

#include "base_types.h"
#include "base_string.h"
#include "base_dynamic_array.h"

using string_builder = dynamic_array<char>;

inline void
sb_write(string_builder *builder, string str)
{
	slice<char> sl = {};
	sl.data = str.data;
	sl.count = str.count;

	array_add_many(builder, sl);
}

inline string
sb_to_string(string_builder builder)
{
	string result = {};
	result.data = builder.data;
	result.count = builder.count;

	return result;
}
