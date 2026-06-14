#pragma once

#include "common.h"

#define TYPE_LIST(X) \
	X(Type_Unknown,     0,    "unknown"    ) \
	X(Type_Void,        1,    "void"       ) \
	X(Type_Int64,       2,    "i64"        ) \
	X(Type_Bool,        3,    "bool"       )

DEFINE_ENUM_WITH_VALUES(Type, u32, TYPE_LIST);
