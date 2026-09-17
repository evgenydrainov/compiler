#pragma once

#include "base/base.h"
#include <stdio.h>

enum PrintFormatSpec : u32
{
	PrintFormatSpec_v,
	PrintFormatSpec_d,
	PrintFormatSpec_u,
	PrintFormatSpec_s,
	PrintFormatSpec_f,
	PrintFormatSpec_X,
	PrintFormatSpec_lld,
	PrintFormatSpec_llu,
	PrintFormatSpec_llX,
};

inline void
WriteRange(FILE *out, char *from, char *to)
{
	fwrite(from, 1, (usize)(to - from), out);
}

inline void
Write(FILE *out, PrintFormatSpec spec, i32 value)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_d)
	{
		fprintf(out, "%d", value);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline void
Write(FILE *out, PrintFormatSpec spec, u32 value)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_u)
	{
		fprintf(out, "%u", value);
	}
	else if (spec == PrintFormatSpec_X)
	{
		fprintf(out, "%X", value);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline void
Write(FILE *out, PrintFormatSpec spec, i64 value)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_lld)
	{
		fprintf(out, "%lld", value);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline void
Write(FILE *out, PrintFormatSpec spec, u64 value)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_llu)
	{
		fprintf(out, "%llu", value);
	}
	else if (spec == PrintFormatSpec_llX)
	{
		fprintf(out, "%llX", value);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline void
Write(FILE *out, PrintFormatSpec spec, f32 value)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_f)
	{
		fprintf(out, "%f", value);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline void
Write(FILE *out, PrintFormatSpec spec, f64 value)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_f)
	{
		fprintf(out, "%f", value);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline void
Write(FILE *out, PrintFormatSpec spec, const char *str)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_s)
	{
		fputs(str, out);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline void
Write(FILE *out, PrintFormatSpec spec, string str)
{
	if (spec == PrintFormatSpec_v || spec == PrintFormatSpec_s)
	{
		WriteRange(out, str.data, str.data + str.count);
	}
	else
	{
		Assert("invalid format specifier for this type");
	}
}

inline bool
WriteUntilPlaceholder(FILE *out,
					  char * &fmt,
					  char *end,
					  PrintFormatSpec &spec)
{
	char *p = fmt;
	while (p != end)
	{
		if (p[0] != '%')
		{
			p++;
			continue;
		}

		WriteRange(out, fmt, p);

		usize rest = (usize)(end - p);
		usize length = 2;

		if (rest >= 2 && p[1] == 'v')
		{
			spec = PrintFormatSpec_v;
		}
		else if (rest >= 2 && p[1] == 'd')
		{
			spec = PrintFormatSpec_d;
		}
		else if (rest >= 2 && p[1] == 'u')
		{
			spec = PrintFormatSpec_u;
		}
		else if (rest >= 2 && p[1] == 's')
		{
			spec = PrintFormatSpec_s;
		}
		else if (rest >= 2 && p[1] == 'f')
		{
			spec = PrintFormatSpec_f;
		}
		else if (rest >= 2 && p[1] == 'X')
		{
			spec = PrintFormatSpec_X;
		}
		else if (rest >= 4 && p[1] == 'l' && p[2] == 'l' && p[3] == 'd')
		{
			spec = PrintFormatSpec_lld;
			length = 4;
		}
		else if (rest >= 4 && p[1] == 'l' && p[2] == 'l' && p[3] == 'u')
		{
			spec = PrintFormatSpec_llu;
			length = 4;
		}
		else if (rest >= 4 && p[1] == 'l' && p[2] == 'l' && p[3] == 'X')
		{
			spec = PrintFormatSpec_llX;
			length = 4;
		}
		else
		{
			Assert(!"unknown format specifier");
		}

		fmt = p + length;

		return true;
	}

	WriteRange(out, fmt, end);
	fmt = end;

	return false;
}

inline void
PrintFormat_Impl(FILE *out,
				 char *fmt,
				 char *end)
{
	PrintFormatSpec spec = {};
	while (WriteUntilPlaceholder(out, fmt, end, spec))
	{
		Assert(!"more placeholders than arguments");
	}
}

template <typename T, typename... Rest>
inline void
PrintFormat_Impl(FILE *out,
				 char *fmt,
				 char *end,
				 T first,
				 Rest... rest)
{
	PrintFormatSpec spec = {};
	bool found = WriteUntilPlaceholder(out, fmt, end, spec);
	if (!found)
	{
		Assert(!"more arguments than placeholders");
	}
	Write(out, spec, first);
	PrintFormat_Impl(out, fmt, end, rest...);
}

template <typename... Args>
inline void
PrintFormat(FILE *out,
			string fmt,
			Args... args)
{
	PrintFormat_Impl(out,
					 fmt.data,
					 fmt.data + fmt.count,
					 args...);
}
