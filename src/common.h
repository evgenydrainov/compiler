#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h> // for memset

//#include <stdio.h>

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

template <typename T, usize N>
inline usize
ArrayCount(const T (&arr)[N])
{
	return N;
}

template <typename T>
inline T
Max(T a, T b)
{
	T result = (a >= b) ? a : b;
	return result;
}

template <typename T>
inline T
Min(T a, T b)
{
	T result = (a < b) ? a : b;
	return result;
}

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
		Assert(index >= 0 && index < count);
		return data[index];
	}

	const char &operator[](usize index) const
	{
		Assert(index >= 0 && index < count);
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

	bool operator!=(const string &other) const
	{
		return !(*this == other);
	}

	char *begin() { return &data[0]; }
	char *end()   { return &data[count]; }
};

#define STR_FMT "%.*s"
#define STR_FMT_QUOTED "'%.*s'"
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
			List(GENERATE_ENUM_NAME)                    \
		}                                               \
		return "unknown";                               \
	}                                                   \
	inline const char *                                 \
	Get##Type##PrettyName(Type value)                   \
	{                                                   \
		switch (value)                                  \
		{                                               \
			List(GENERATE_ENUM_PRETTY_NAME)             \
		}                                               \
		return "unknown";                               \
	}

#define GENERATE_ENUM(Name, Value, PrettyName) Name=Value,
#define GENERATE_ENUM_NAME(Name, Value, PrettyName) case Name: return #Name;
#define GENERATE_ENUM_PRETTY_NAME(Name, Value, PrettyName) case Name: return PrettyName;

string LoadFile(const char *fileName);
string LoadFile(string fileName);

struct Arena
{
	u8 *data;
	usize pos;
	usize capacity;
};

#define DEFAULT_ALIGNMENT (sizeof(void *))

inline bool
IsPowerOfTwo(usize x)
{
	bool result = ((x != 0)
				   && ((x & (x - 1)) == 0));
	return result;
}

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
	usize alignedPos = AlignForward(arena->pos, alignment);

	Assert(alignedPos + size <= arena->capacity);

	void *result = arena->data + alignedPos;
	arena->pos = alignedPos + size;

	memset(result, 0, size);

	/*f32 percentage = arena->pos/(f32)arena->capacity;
	printf("%f\n", percentage);
	if (percentage > 0.90)
	{
		int k = 123;
	}*/

	return result;
}

template <typename T>
inline T *
PushStruct(Arena *arena)
{
	T *result = (T *)PushSize(arena, sizeof(T));
	return result;
}

template <typename T>
inline T *
PushArray(Arena *arena, usize count)
{
	T *result = (T *)PushSize(arena, count*sizeof(T));
	return result;
}

inline Arena
PushArena(Arena *arena, usize capacity)
{
	Arena result = {};
	result.data = (u8 *)PushSize(arena, capacity);
	result.capacity = capacity;

	return result;
}

template <typename T>
struct ExitScope
{
	T lambda;
	ExitScope(T lambda) : lambda(lambda) {}
	~ExitScope() { lambda(); }
	ExitScope(const ExitScope&);
private:
	ExitScope &operator=(const ExitScope&);
};

struct ExitScopeHelp
{
	template <typename T>
	ExitScope<T> operator+(T t) { return t; }
};

#define defer const auto &CONCATENATE(_defer, __LINE__) = ExitScopeHelp() + [&]()

#define CONCATENATE2(a, b) a##b
#define CONCATENATE(a, b) CONCATENATE2(a, b)

template <typename T, typename U>
struct IsSameType
{
	static constexpr bool value = false;
};
template <typename T>
struct IsSameType<T, T>
{
	static constexpr bool value = true;
};

template <typename T>
struct BumpArray
{
	T *data;
	usize count;
	usize capacity;

	T &operator[](usize index)
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	const T &operator[](usize index) const
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	T *begin() { return &data[0]; }
	T *end()   { return &data[count]; }
};

template <typename T>
inline void
ArrayAdd(BumpArray<T> *array, const T &value)
{
	Assert(array->count < array->capacity);

	array->data[array->count] = value;
	array->count++;
}

template <typename T>
inline BumpArray<T>
PushBumpArray(Arena *arena, usize capacity)
{
	BumpArray<T> result = {};
	result.data = PushArray<T>(arena, capacity);
	result.capacity = capacity;

	return result;
}

template <typename T, usize capacity>
struct StaticBumpArray
{
	T data[capacity];
	usize count;

	T &operator[](usize index)
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	const T &operator[](usize index) const
	{
		Assert(index >= 0 && index < count);
		return data[index];
	}

	T *begin() { return &data[0]; }
	T *end()   { return &data[count]; }
};

template <typename T, usize capacity>
inline T *
ArrayAdd(StaticBumpArray<T, capacity> *array, const T &value)
{
	Assert(array->count < capacity);

	T *result = &array->data[array->count];

	*result = value;
	array->count++;

	return result;
}

inline bool
IsSpace(char c)
{
	bool result = (c == ' '
				   || c == '\n'
				   || c == '\r'
				   || c == '\t');
	return result;
}

inline void
TrimLeft(string *str)
{
	while (str->count > 0
		   && IsSpace(str->data[0]))
	{
		str->data++;
		str->count--;
	}
}

inline bool
StartsWith(string str, string prefix)
{
	bool result = false;

	if (str.count >= prefix.count)
	{
		str.count = prefix.count;
		result = (str == prefix);
	}

	return result;
}
