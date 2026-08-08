#pragma once

#include <stdint.h>
#include <stddef.h>

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

#define CONCATENATE2(a, b) a##b
#define CONCATENATE(a, b) CONCATENATE2(a, b)

inline usize
Kilobytes(usize N)
{
	usize result = N<<10;
	return result;
}

inline usize
Megabytes(usize N)
{
	usize result = N<<20;
	return result;
}

inline usize
Gigabytes(usize N)
{
	usize result = N<<30;
	return result;
}

void AssertionHandler(char *file, int line, char *condition);

#define Assert(condition) ((condition) ? (void)0 : AssertionHandler(__FILE__, __LINE__, #condition))

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

inline void *
MemSet(void *dest, int value, usize count)
{
	u8 *dest8 = (u8 *)dest;
	while (count--)
	{
		*dest8++ = (u8)value;
	}

	return dest;
}

inline void *
MemCpy(void *dest, void *src, usize count)
{
	u8 *dest8 = (u8 *)dest;
	u8 *src8 = (u8 *)src;
	while (count--)
	{
		*dest8++ = *src8++;
	}

	return dest;
}

inline usize
StrLen(char *str)
{
	usize result = 0;
	while (*str++)
	{
		result++;
	}

	return result;
}

inline bool
is_power_of_two(usize x)
{
	bool result = (x != 0) && ((x & (x - 1)) == 0);
	return result;
}

inline usize
align_forward(usize ptr, usize alignment)
{
	Assert(is_power_of_two(alignment));
	return (ptr + (alignment-1)) & ~(alignment-1);
}
