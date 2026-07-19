#pragma once

#include "common_types.h"

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
