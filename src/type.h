#pragma once

#include "common.h"

#define TYPE_KIND_LIST(X) \
	X(TypeKind_Unknown,     0,    "unknown type"    ) \
	X(TypeKind_Void,        1,    "void"            ) \
	X(TypeKind_Int64,       2,    "i64"             ) \
	X(TypeKind_Bool,        3,    "bool"            ) \
	X(TypeKind_Pointer,     4,    "pointer"         ) \
	X(TypeKind_Struct,      5,    "struct"          )

DEFINE_ENUM_WITH_VALUES(TypeKind, u32, TYPE_KIND_LIST);

struct Type
{
	TypeKind kind;
	Type *pointerTo;
};

inline bool
TypesEqual(Type a, Type b)
{
	if (a.kind == TypeKind_Pointer
		&& b.kind == TypeKind_Pointer)
	{
		return TypesEqual(*a.pointerTo, *b.pointerTo);
	}

	return a.kind == b.kind;
}
