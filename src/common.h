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

#define Kilobytes(n) ((n)*1024LL)
#define Megabytes(n) (Kilobytes(n)*1024LL)
#define Gigabytes(n) (Megabytes(n)*1024LL)

#define Assert(cond) ((cond) ? (void)0 : __debugbreak())

#define ArrayCount(array) (sizeof(array)/sizeof((array)[0]))

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

struct Arena
{
	u8 *data;
	usize pos;
	usize capacity;
};

#define DEFAULT_ALIGNMENT (sizeof(void *))

#define IsPowerOfTwo(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)

inline usize
AlignForward(usize ptr, usize alignment)
{
	Assert(IsPowerOfTwo(alignment));
	return (ptr + (alignment-1)) & ~(alignment-1);
}

inline void *
PushSize(Arena *arena,
		 usize size,
		 usize alignment = DEFAULT_ALIGNMENT)
{
	//
	// Assume that arena->data is already aligned
	//

	usize alignedPos = AlignForward(arena->pos, alignment);

	Assert(alignedPos + size <= arena->capacity);

	void *result = arena->data + alignedPos;
	arena->pos = alignedPos + size;

	return result;
}

#define PushStruct(arena, T) (T*)PushSize(arena, sizeof(T))

#define PushArray(arena, count, T) (T*)PushSize(arena, (count)*sizeof(T))

inline Arena
PushArena(Arena *arena, usize capacity)
{
	Arena result = {};
	result.data = (u8 *)PushSize(arena, capacity);
	result.capacity = capacity;

	return result;
}
