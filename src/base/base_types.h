#pragma once

#include <stdint.h>
#include <stddef.h>

#define internal static
#define local_persist static
#define global_variable static

#define foreach(it, array) for (auto it = (array).begin(); it != (array).end(); it++)

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

#define CONCATENATE_IMPL(a, b) a##b
#define CONCATENATE(a, b) CONCATENATE_IMPL(a, b)

inline usize
kilobytes(usize N)
{
	usize result = N<<10;
	return result;
}

inline usize
megabytes(usize N)
{
	usize result = N<<20;
	return result;
}

inline usize
gigabytes(usize N)
{
	usize result = N<<30;
	return result;
}

template <typename T, usize N>
inline usize
array_length(const T (&arr)[N])
{
	return N;
}

template <typename T, typename U>
struct is_same
{
	static constexpr bool value = false;
};

template <typename T>
struct is_same<T, T>
{
	static constexpr bool value = true;
};

template <typename T>
struct type_identity
{
	typedef T type;
};

#define IDENTITY(Type) typename type_identity<Type>::type

#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_BLACK   "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"

#define NO_STEP_INTO __declspec(non_user_code)
