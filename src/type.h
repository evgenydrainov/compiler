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

struct StructInfo;

struct Type
{
	TypeKind kind;
	string name;
	Type *pointerTo;
	StructInfo *structInfo;
};

struct StructField
{
	string name;
	Type type;
	int offset;
};

struct StructInfo
{
	string name;
	StructField fields[32];
	int numFields;
	int size;
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

inline int
SizeOfType(Type type)
{
	if (type.kind == TypeKind_Struct)
	{
		return type.structInfo->size;
	}
	return 8;
}

inline StructField *
FindField(StructInfo *info, string name)
{
	StructField *result = nullptr;

	for (int i = 0;
		 i < info->numFields;
		 i++)
	{
		if (info->fields[i].name == name)
		{
			result = &info->fields[i];
			break;
		}
	}

	return result;
}
