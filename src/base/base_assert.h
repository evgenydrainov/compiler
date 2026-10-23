#pragma once

#include "base_types.h"

#ifndef ENABLE_ASSERTIONS
#  ifdef NDEBUG
#    define ENABLE_ASSERTIONS 1 // enable in release for now
#  else
#    define ENABLE_ASSERTIONS 1
#  endif
#endif

#if ENABLE_ASSERTIONS
#  define Assert(condition) ((condition) ? (void)0 : AssertionHandler(__FILE__, __LINE__, #condition))
#else
#  define Assert(condition) ((void)0)
#endif

void AssertionHandler(char *file, int line, char *condition);
