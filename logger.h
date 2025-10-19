#ifndef _LOGGER_H_
#define _LOGGER_H_
#include "base.h"
#include <stdio.h>
#include <linux/limits.h>

#define LOG_NAME "ssu_cleanupd.log"
#define BUFFER_SIZE 4096
typedef struct {
	char path[PATH_MAX];
	int max_log_lines;
} LOG;

extern LOG* logger_open PARAMS((const char*,int,LOG*));
extern int logger_append PARAMS((LOG*, const char*)); 
extern int logger_flush PARAMS((LOG*));
extern int logger_resize PARAMS((LOG*, int));
extern int logger_get PARAMS((LOG*, char**));
extern int logger_nget PARAMS((LOG*, int, char[][BUFFER_SIZE]));
extern void logger_free PARAMS((LOG*));

#endif
