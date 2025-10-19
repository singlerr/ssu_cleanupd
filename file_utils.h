#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#include "base.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

extern FILE* fileutils_open_or_create PARAMS((const char* pathname));
extern int fileutils_lock PARAMS((const char*));
extern int fileutils_unlock PARAMS((int));
#endif
