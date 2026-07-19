#pragma once

#include "common_types.h"
#include "common_arena.h"

#define STR_FMT "%.*s"
#define STR_FMT_QUOTED "'%.*s'"
#define STR_ARG(str) (int)(str).count, (str).data

struct string
{
	char *data;
	usize count;

	string() = default;

	string(char *data, usize count) : data(data), count(count) {}

	template <usize N>
	string(const char (&str)[N]) : data((char *)str), count(N-1) {}

	char &operator[](usize index)
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	const char &operator[](usize index) const
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	bool operator==(const string &other) const
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

	bool operator!=(const string &other) const
	{
		return !(*this == other);
	}

	char *begin() { return &data[0]; }
	char *end()   { return &data[count]; }
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

inline string
trim_left(string str)
{
	while (str.count > 0
		   && is_whitespace(str[0]))
	{
		str.data++;
		str.count--;
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
string_concat(string a, string b)
{
	string result;
	result.count = a.count + b.count;
	result.data = (char *)PushSize(&g_tempMemory, result.count);

	MemCpy(result.data, a.data, a.count);

	MemCpy(result.data + a.count, b.data, b.count);

	return result;
}

inline char *
to_cstring(string str)
{
	char *result = (char *)PushSize(&g_tempMemory, str.count + 1);

	MemCpy(result, str.data, str.count);

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
