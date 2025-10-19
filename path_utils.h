#ifndef _PATH_UTILS_H_
#define _PATH_UTILS_H_
#include "base.h"
#include <dirent.h>
#include <linux/limits.h>
#define VAL_OUTHOME 1
#define VAL_NOTEXIST 1 << 1
#define VAL_NONDIR 1 << 2
#define VAL_IACCESS 1 << 3

extern char* pathutils_real_path PARAMS((const char*, char*));
extern char* pathutils_join PARAMS((const char*, const char*, char*));
extern char* pathutils_get_extension PARAMS((const char*, char*));
extern int pathutils_copy PARAMS((const char*, const char*));
extern int pathutils_mkdir_r PARAMS((const char*));
extern int pathutils_mkdir PARAMS((const char*));
extern void pathutils_free_dirent PARAMS((struct dirent**, int));
extern int pathutils_file_exists PARAMS((const char*));
extern int pathutils_validate PARAMS((const char*));
#endif
