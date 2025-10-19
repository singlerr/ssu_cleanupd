#ifndef _WORKER_GROUP_H_
#define _WORKER_GROUP_H_

#include "base.h"
#include "worker.h"
#include "list.h"

#include <linux/limits.h>

// current working daemon process list
// and manage current_deamon_list
typedef struct _wrkgrp {
	char path[PATH_MAX];
	LIST* workers;
} WRKGRP;

extern WRKGRP* workergroup_create PARAMS((const char*));
extern int workergroup_add PARAMS((WRKGRP*, const WORKER*));
extern int workergroup_remove PARAMS((WRKGRP*, const WORKER*));
extern void workergroup_free PARAMS((WRKGRP*));
extern void workergroup_close PARAMS((WRKGRP*));
#endif
