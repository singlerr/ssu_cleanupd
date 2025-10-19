#include "api.h"
#include "conf.h"
#include "path_utils.h"
#include "logger.h"
#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

char* r_dirpath;
char r_absolute_path[PATH_MAX];

int r_validate_path();

// print usage
void remove_help(int detail){
	if(! detail){
		printf(H_TAB "> remove <DIR_PATH>" H_RT);
	}else{
		printf(H_TAB "> remove <DIR_PATH>" H_RT);
		printf(H_TTAB "<none> : remove deamon process monitoring the <DIR_PATH> directory" H_RT);
	}
}

static char* r_filter_name;

// filter worker with path
int r_filter_path(void* e){
	if(r_filter_name == NULL)
		return false;

	WORKER* w = (WORKER*) e;
	return strcmp(w->conf->monitoring_path, r_filter_name) == 0;
}

int api_remove(int argc, char** argv){
	if(argc < 2){
		remove_help(false);
		return -1;
	}

	r_dirpath = argv[1];
	strcpy(r_absolute_path, r_dirpath);

	r_filter_name = r_absolute_path; 
	NODE *node = list_find_node(wrkgrp->workers, r_filter_path);
	r_filter_name = NULL;
	
	if(node == NULL)
		return -1;

	WORKER* w = (WORKER*) node->value;
	list_remove(wrkgrp->workers, node);
	workergroup_remove(wrkgrp, w);
	int pid = w->conf->pid;
	if(w->conf->pid <= 0)
		return -1;
	
	if(kill(w->conf->pid, SIGKILL) < 0){
		return -1;
	}

	worker_free(w);
	return 0;
}

// check input path is valid on condition
int r_validate_path(){
	pathutils_real_path(r_dirpath, r_absolute_path);

	int vflg = pathutils_validate(r_absolute_path);

	if(vflg & VAL_NOTEXIST)
		return -1;
	if(vflg & VAL_NONDIR)
		return -1;
	if(vflg & VAL_OUTHOME){
		printf("%s is outside the home directory\n", r_dirpath);
		return -1;
	}

	return 0;
}
