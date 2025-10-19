#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_
#include "base.h"
#include <stdio.h>

extern char* stringutils_replace PARAMS((const char*, const char*, const char*, char *)); 
extern size_t stringutils_strcnt PARAMS((const char* s, char c));
extern char* stringutils_consume_or_null PARAMS((const char*, const char*));
extern char* stringutils_find_any PARAMS((const char**, size_t, const char*, int (*)(const char*, const char*)));
extern int stringutils_startwith PARAMS((const char*, const char*));
extern int stringutils_strstartwith PARAMS((const char*, const char*));
extern int stringutils_strequal PARAMS((const char*, const char*));
#endif
