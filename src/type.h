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
	X(TypeKind_Enum,       11,    "enum"             ) \
	X(TypeKind_Array,      12,    "array"            )

DEFINE_ENUM_WITH_VALUES(TypeKind, u32, TYPE_KIND_LIST);

struct StructInfo;
struct EnumInfo;
struct Node;

struct Type
{
	TypeKind kind;
	string name;
	Type *pointerTo;
	StructInfo *structInfo;
	EnumInfo *enumInfo;

	Type *arrayElementType;
	Node *arrayLengthExpr;
	int arrayLength;
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

struct EnumeratorInfo
{
	string name;
	i64 value;
};

struct EnumInfo
{
	string name;
	StaticBumpArray<EnumeratorInfo, 32> enumerators;
};

inline bool
TypesEqual(Type a, Type b)
{
	if (a.kind == TypeKind_Pointer
		&& b.kind == TypeKind_Pointer)
	{
		return TypesEqual(*a.pointerTo, *b.pointerTo);
	}

	if (a.kind == TypeKind_Struct
		&& b.kind == TypeKind_Struct)
	{
		return a.name == b.name;
	}

	return a.kind == b.kind;
}

inline int
SizeOfType(Type type)
{
	int result = 0;

	switch (type.kind)
	{
		case TypeKind_Int8:    {result = 1;} break;
		case TypeKind_Int16:   {result = 2;} break;
		case TypeKind_Int32:   {result = 4;} break;
		case TypeKind_Int64:   {result = 8;} break;
		case TypeKind_Pointer: {result = 8;} break;
		case TypeKind_Bool:    {result = 8;} break;
		case TypeKind_Enum:    {result = 8;} break;

		case TypeKind_Struct:
		{
			Assert(type.structInfo && "type was not resolved");

			result = type.structInfo->size;
		} break;

		case TypeKind_Array:
		{
			Assert(type.arrayLength != 0 && "type was not resolved");

			int elementSize = SizeOfType(*type.arrayElementType);
			result = type.arrayLength * elementSize;
		} break;

		case TypeKind_Unknown: {} break;

		default:
		{
			Assert(false);
		} break;
	}

	return result;
}

inline StructField *
FindField(StructInfo *info, string name)
{
	Assert(info && "type was not resolved");

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

inline EnumeratorInfo *
FindEnumerator(EnumInfo *info, string name)
{
	Assert(info && "type was not resolved");

	EnumeratorInfo *result = nullptr;

	for (usize i = 0;
		 i < info->enumerators.count;
		 i++)
	{
		if (info->enumerators[i].name == name)
		{
			result = &info->enumerators[i];
			break;
		}
	}

	return result;
}
