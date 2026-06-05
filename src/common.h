#pragma once

#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;
typedef ptrdiff_t isize;

typedef float f32;
typedef double f64;

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
		return data[index];
	}

	const char &operator[](usize index) const
	{
		return data[index];
	}
};

#define STR_FMT "%.*s"
#define STR_ARG(str) (int)(str).count, (str).data

#define DEFINE_ENUM_WITH_VALUES(Type, Underlying, List) \
	enum Type : Underlying                              \
	{                                                   \
		List(GENERATE_ENUM)                             \
	};                                                  \
	inline const char *                                 \
	Get##Type##Name(Type value)                         \
	{                                                   \
		switch (value)                                  \
		{                                               \
			List(GENERATE_ENUM_CASE)                    \
		}                                               \
		return "unknown";                               \
	}

#define GENERATE_ENUM(Name, Value) Name=Value,
#define GENERATE_ENUM_CASE(Name, Value) case Name: return #Name;
