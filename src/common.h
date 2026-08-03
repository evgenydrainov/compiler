#pragma once

#include "common_arena.h"
#include "common_array.h"
#include "common_defer.h"
#include "common_file.h"
#include "common_string.h"
#include "common_types.h"

#define DEFINE_ENUM_WITH_VALUES(Type, Underlying, List) \
	enum Type : Underlying                              \
	{                                                   \
		List(GENERATE_ENUM)                             \
	};                                                  \
	inline char *                                       \
	Get##Type##Name(Type value)                         \
	{                                                   \
		switch (value)                                  \
		{                                               \
			List(GENERATE_ENUM_NAME)                    \
		}                                               \
		return "invalid enum value";                    \
	}                                                   \
	inline char *                                       \
	Get##Type##PrettyName(Type value)                   \
	{                                                   \
		switch (value)                                  \
		{                                               \
			List(GENERATE_ENUM_PRETTY_NAME)             \
		}                                               \
		return "invalid enum value";                    \
	}

#define GENERATE_ENUM(Name, Value, PrettyName) Name=Value,
#define GENERATE_ENUM_NAME(Name, Value, PrettyName) case Name: return #Name;
#define GENERATE_ENUM_PRETTY_NAME(Name, Value, PrettyName) case Name: return PrettyName;
