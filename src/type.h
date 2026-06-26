#pragma once

#include "common.h"

#define TYPE_KIND_LIST(X) \
	X(TypeKind_Unknown,     0,    "unknown type"     ) \
	X(TypeKind_InferMe,     1,    "not inferred yet" ) \
	X(TypeKind_Void,        2,    "void"             ) \
	X(TypeKind_Int8,        3,    "i8"               ) \
	X(TypeKind_Int16,       4,    "i16"              ) \
	X(TypeKind_Int32,       5,    "i32"              ) \
	X(TypeKind_Int64,       6,    "i64"              ) \
	X(TypeKind_Bool,        7,    "bool"             ) \
	X(TypeKind_Pointer,     8,    "pointer"          ) \
	X(TypeKind_Struct,      9,    "struct"           ) \
	X(TypeKind_String,     10,    "string"           )

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
		if (type.structInfo)
		{
			return type.structInfo->size;
		}
		else
		{
			// type was not resolved
			return 0;
		}
	}

	if (type.kind == TypeKind_Int8)
	{
		return 1;
	}

	if (type.kind == TypeKind_Int16)
	{
		return 2;
	}

	if (type.kind == TypeKind_Int32)
	{
		return 4;
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
