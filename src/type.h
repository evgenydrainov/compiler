#pragma once

#include "common.h"

#define TYPE_KIND_LIST(X) \
	X(TypeKind_Unknown,       0,    "unknown type"     ) \
	X(TypeKind_InferMe,       1,    "not inferred yet" ) \
	X(TypeKind_Void,          2,    "void"             ) \
	/* integer types */ \
	X(TypeKind_Int8,          3,    "i8"               ) \
	X(TypeKind_Int16,         4,    "i16"              ) \
	X(TypeKind_Int32,         5,    "i32"              ) \
	X(TypeKind_Int64,         6,    "i64"              ) \
	/* unsigned integer types */ \
	X(TypeKind_UInt8,         7,    "u8"               ) \
	X(TypeKind_UInt16,        8,    "u16"              ) \
	X(TypeKind_UInt32,        9,    "u32"              ) \
	X(TypeKind_UInt64,       10,    "u64"              ) \
	/* floating point types */ \
	X(TypeKind_Float32,      11,    "f32"              ) \
	X(TypeKind_Float64,      12,    "f64"              ) \
	/* other */ \
	X(TypeKind_Bool,         13,    "bool"             ) \
	X(TypeKind_Pointer,      14,    "pointer"          ) \
	X(TypeKind_Struct,       15,    "struct"           ) \
	X(TypeKind_Enum,         16,    "enum"             ) \
	X(TypeKind_Array,        17,    "array"            ) \
	X(TypeKind_Slice,        18,    "slice"            ) \
	X(TypeKind_DynamicArray, 19,    "dynamic array"    )

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
	StaticBumpArray<StructField, 32> fields;
	int size;
	int alignment;
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

	if (a.kind == TypeKind_Array
		&& b.kind == TypeKind_Array)
	{
		return (a.arrayLength != 0
				&& a.arrayLength == b.arrayLength
				&& TypesEqual(*a.arrayElementType, *b.arrayElementType));
	}

	if (a.kind == TypeKind_Slice
		&& b.kind == TypeKind_Slice)
	{
		return TypesEqual(*a.arrayElementType, *b.arrayElementType);
	}

	if (a.kind == TypeKind_DynamicArray
		&& b.kind == TypeKind_DynamicArray)
	{
		return TypesEqual(*a.arrayElementType, *b.arrayElementType);
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

		case TypeKind_UInt8:   {result = 1;} break;
		case TypeKind_UInt16:  {result = 2;} break;
		case TypeKind_UInt32:  {result = 4;} break;
		case TypeKind_UInt64:  {result = 8;} break;

		case TypeKind_Float32: {result = 4;} break;
		case TypeKind_Float64: {result = 8;} break;

		case TypeKind_Pointer:      {result = 8;} break;
		case TypeKind_Bool:         {result = 1;} break;
		case TypeKind_Enum:         {result = 8;} break;
		case TypeKind_Slice:        {result = 16;} break;
		case TypeKind_DynamicArray: {result = 24;} break;

		// NOTE: the size of a void type is asked when a function checks if
		// it has a large struct return value
		case TypeKind_Void:    {result = 1;} break;

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

		case TypeKind_Unknown:
		case TypeKind_InferMe:
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

	for (usize i = 0;
		 i < info->fields.count;
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

inline bool
IsSignedInteger(Type type)
{
	return (type.kind == TypeKind_Int64
			|| type.kind == TypeKind_Int32
			|| type.kind == TypeKind_Int16
			|| type.kind == TypeKind_Int8);
}

inline bool
IsUnsignedInteger(Type type)
{
	return (type.kind == TypeKind_UInt64
			|| type.kind == TypeKind_UInt32
			|| type.kind == TypeKind_UInt16
			|| type.kind == TypeKind_UInt8);
}

inline bool
IsInteger(Type type)
{
	return (IsSignedInteger(type)
			|| IsUnsignedInteger(type));
}

inline bool
IsFloatingPoint(Type type)
{
	return (type.kind == TypeKind_Float32
			|| type.kind == TypeKind_Float64);
}

inline bool
IsRegisterSized(Type type)
{
	int size = SizeOfType(type);
	return (size == 1
			|| size == 2
			|| size == 4
			|| size == 8);
}

inline int
AlignmentOfType(Type type)
{
	int result = 0;

	switch (type.kind)
	{
		case TypeKind_Struct:
		{
			Assert(type.structInfo && "type was not resolved");
			result = type.structInfo->alignment;
		} break;

		case TypeKind_Array:
		{
			Assert(type.arrayLength != 0 && "type was not resolved");
			result = AlignmentOfType(*type.arrayElementType);
		} break;

		case TypeKind_Slice:        {result = 8;} break;
		case TypeKind_DynamicArray: {result = 8;} break;

		default:
		{
			result = SizeOfType(type);
		} break;
	}

	Assert(result == 1
		   || result == 2
		   || result == 4
		   || result == 8);

	return result;
}
