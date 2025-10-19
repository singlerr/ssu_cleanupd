#ifndef _CONF_H_
#define _CONF_H_

#include "base.h"
#include <linux/limits.h>
#include <time.h>

#define CONF_NONE "none"
#define CONF_ALL "all"
#define CONF_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define END_OF_LINE(str, len, p, fp) \
	do { \
		p = strchr(str, '\n'); \
		if(p == NULL){ \
			p = fp + len; \
		} \
	} while(0);
#define NOT_NULL(p) \
	do { \
		if(p == NULL) \
			return NULL; \
	} while(0);
#define NOT_NEG(v) \
	do { \
		if(p < 0) \
			return NULL; \
	} while(0);
#define STR_LEN(np, p) (np - p) / sizeof(char)

typedef enum {
	INCLUDE_LATEST = 1,
	INCLUDE_OLDEST = 2,
	EXCLUDE = 3
} DUP_STRATEGY;

typedef struct {
	char conf_path[PATH_MAX];	
	char monitoring_path[PATH_MAX];
	int pid;
	time_t start_time;
	char output_path[PATH_MAX];
	int time_interval;
	int max_log_lines;
	size_t exclude_path_count;
	char exclude_path[10][PATH_MAX];
	size_t extension_count;
	char extension[10][20];
	DUP_STRATEGY mode;
} CONF;

extern char* conf_serialize PARAMS((const CONF*, char*));
extern CONF* conf_deserialize PARAMS((const char*, CONF*));
extern CONF* conf_open PARAMS((const int, CONF*));
extern int conf_flush PARAMS((const CONF*, const char*));
extern void conf_free PARAMS((CONF*));
#endif
