#pragma once

#include "base_types.h"
#include "base_slice.h"
#include "base_allocator.h"
#include <string.h> // for memcpy

#define STR_FMT "%.*s"
#define STR_FMT_QUOTED "'%.*s'"
#define STR_ARG(str) (int)(str).count, (str).data

struct string
{
	char *data;
	usize count;

	NO_STEP_INTO string() = default;

	NO_STEP_INTO string(char *data, usize count) : data(data), count(count) {}

	template <usize N>
	NO_STEP_INTO string(const char (&str)[N]) : data((char *)str), count(N-1) {}

	NO_STEP_INTO char &operator[](usize index)
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	NO_STEP_INTO const char &operator[](usize index) const
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	NO_STEP_INTO bool operator==(const string &other) const
	{
		if (count != other.count)
		{
			return false;
		}

		for (usize i = 0; i < count; i++)
		{
			if (data[i] != other.data[i])
			{
				return false;
			}
		}

		return true;
	}

	NO_STEP_INTO bool operator!=(const string &other) const
	{
		return !(*this == other);
	}

	NO_STEP_INTO char *begin() { return &data[0]; }
	NO_STEP_INTO char *end()   { return &data[count]; }

	NO_STEP_INTO operator bool() const
	{
		return count != 0;
	}
};

inline bool
is_whitespace(char c)
{
	bool result = (c == ' '
				   || c == '\n'
				   || c == '\r'
				   || c == '\t');
	return result;
}

inline void
string_advance(string *str, usize count = 1)
{
	Assert(str->count >= count);

	str->count -= count;
	str->data += count;
}

inline string
trim_left(string str)
{
	while (str && is_whitespace(str[0]))
	{
		string_advance(&str);
	}

	return str;
}

inline bool
starts_with(string str, string prefix)
{
	bool result = false;

	if (str.count >= prefix.count)
	{
		str.count = prefix.count;
		result = (str == prefix);
	}

	return result;
}

inline string
strip_filename(string filepath)
{
	for (usize i = filepath.count;
		 i--;)
	{
		if (filepath[i] == '/'
			|| filepath[i] == '\\')
		{
			filepath.count = i + 1;
			return filepath;
		}
	}

	return {};
}

inline string
string_concat(string a, string b, Allocator allocator)
{
	string result = {};
	result.count = a.count + b.count;
	result.data = (char *)mem_alloc(result.count, DEFAULT_ALIGNMENT, allocator);

	memcpy(result.data, a.data, a.count);

	memcpy(result.data + a.count, b.data, b.count);

	return result;
}

inline char *
to_cstring(string str, Allocator allocator)
{
	char *result = (char *)mem_alloc(str.count + 1, DEFAULT_ALIGNMENT, allocator);
	memcpy(result, str.data, str.count);
	result[str.count] = 0;

	return result;
}

inline string
strip_extension(string filepath)
{
	for (usize i = filepath.count;
		 i--;)
	{
		if (filepath[i] == '.')
		{
			filepath.count = i;
			return filepath;
		}

		if (filepath[i] == '/'
			|| filepath[i] == '\\')
		{
			return filepath;
		}
	}

	return filepath;
}

inline bool
is_path_absolute(string path)
{
	if (path.count >= 1)
	{
		if (path[0] == '/' || path[0] == '\\')
		{
			return true;
		}
	}

	if (path.count >= 3)
	{
		if (path[1] == ':'
			&& (path[2] == '/' || path[2] == '\\'))
		{
			return true;
		}
	}

	return false;
}

inline bool
string_needs_quoting(string str)
{
	bool insideQuote = false;

	for (char it : str)
	{
		if (it == '"')
		{
			insideQuote ^= true;
		}

		if (it == ' ')
		{
			if (!insideQuote)
			{
				return true;
			}
		}
	}

	return false;
}

template <>
inline slice<char>::slice(string str)
	: data(str.data), count(str.count)
{
}

inline string
string_slice(string str, isize from, isize to)
{
	if (from < 0)
	{
		from += str.count;
	}

	if (to < 0)
	{
		to += str.count;
	}

	Assert(from >= 0 && from <= (isize)str.count);
	Assert(to >= 0 && to <= (isize)str.count);
	Assert(from <= to);

	string result = {};
	result.data = str.data + from;
	result.count = to - from;

	return result;
}


