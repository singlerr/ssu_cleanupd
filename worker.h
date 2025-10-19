#ifndef _WORKER_H_
#define _WORKER_H_

#include "base.h"
#include "conf.h"
#include "logger.h"
#include <linux/limits.h>

#define CONF_NAME "ssu_cleanupd.config"

// struct representing daemon process
typedef struct {
	char path[PATH_MAX];
	CONF *conf;
	LOG *log;
} WORKER;

extern int worker_start PARAMS((const WORKER*));
extern void worker_free PARAMS((WORKER*));

#endif
