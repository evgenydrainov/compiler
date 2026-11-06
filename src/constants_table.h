#pragma once

#include "base/base.h"

// TODO: why not just use TypeKind?
enum ConstantKind : u32
{
	ConstantKind_Int64,
	ConstantKind_Float32,
	ConstantKind_Float64,
	ConstantKind_Bool,
};

struct ConstantValue
{
	ConstantKind kind;
	union
	{
		i64 int64Value;
		f32 float32Value;
		f64 float64Value;
		bool boolValue;
	};
};

struct Constant
{
	string name;
	ConstantValue value;
};

struct ConstantsTable
{
	dynamic_array<Constant> constants;
};

inline Constant *
LookupConstant(ConstantsTable *table,
			   string name)
{
	for (auto &it : table->constants)
	{
		if (it.name == name)
		{
			return &it;
		}
	}

	return nullptr;
}

inline Constant *
DeclareConstant(ConstantsTable *table,
				string name)
{
	Constant *result = array_add(&table->constants, {});
	result->name = name;

	return result;
}

inline i64
AsInt64(ConstantValue value)
{
	switch (value.kind)
	{
		case ConstantKind_Int64:   return value.int64Value;
		case ConstantKind_Float32: return (i64)value.float32Value;
		case ConstantKind_Float64: return (i64)value.float64Value;
		case ConstantKind_Bool:    return (value.boolValue ? 1 : 0);
	}

	Assert(false);
	return 0;
}

inline f32
AsFloat32(ConstantValue value)
{
	switch (value.kind)
	{
		case ConstantKind_Int64:   return (f32)value.int64Value;
		case ConstantKind_Float32: return value.float32Value;
		case ConstantKind_Float64: return (f32)value.float64Value;
		case ConstantKind_Bool:    return (value.boolValue ? 1.0f : 0.0f);
	}

	Assert(false);
	return 0;
}

inline f64
AsFloat64(ConstantValue value)
{
	switch (value.kind)
	{
		case ConstantKind_Int64:   return (f64)value.int64Value;
		case ConstantKind_Float32: return (f64)value.float32Value;
		case ConstantKind_Float64: return value.float64Value;
		case ConstantKind_Bool:    return (value.boolValue ? 1.0 : 0.0);
	}

	Assert(false);
	return 0;
}

inline f64
AsBool(ConstantValue value)
{
	switch (value.kind)
	{
		case ConstantKind_Int64:   return (value.int64Value != 0);
		case ConstantKind_Float32: return (value.float32Value != 0);
		case ConstantKind_Float64: return (value.float64Value != 0);
		case ConstantKind_Bool:    return value.boolValue;
	}

	Assert(false);
	return 0;
}

inline char *
GetConstantKindName(ConstantKind kind)
{
	switch (kind)
	{
		case ConstantKind_Int64:   return "i64";
		case ConstantKind_Float32: return "f32";
		case ConstantKind_Float64: return "f64";
		case ConstantKind_Bool:    return "bool";
	}

	Assert(false);
	return "<unknown type>";
}

inline TypeKind
ConstantKindToTypeKind(ConstantKind kind)
{
	switch (kind)
	{
		case ConstantKind_Int64:   return TypeKind_Int64;
		case ConstantKind_Float32: return TypeKind_Float32;
		case ConstantKind_Float64: return TypeKind_Float64;
		case ConstantKind_Bool:    return TypeKind_Bool;
	}

	Assert(false);
	return {};
}
