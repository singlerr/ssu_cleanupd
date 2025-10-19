#include "worker_group.h"
#include "path_utils.h"
#include "file_utils.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>


#define BUFFER_SIZE 4096
// serialize paths of daemon processes into string
char* workergroup_serialize(const WRKGRP*, char*);
// deserialize paths of daemon processes
WRKGRP* workergroup_deserialize(const char*, WRKGRP*);

// create new worker group and deserialize current_deamon_list and start deamon processes
WRKGRP* workergroup_create(const char* path){
	char buf[BUFFER_SIZE];
	char raw[BUFFER_SIZE];
	int fd;

	WRKGRP* w = (WRKGRP*) malloc(sizeof(WRKGRP));
	w->workers = list_create();
	strcpy(w->path, path);

	if((fd = open(path, O_RDWR | O_CREAT, S_IRWXU)) < 0)
		return NULL;

	lseek(fd, 0, SEEK_SET);
	size_t len;
	size_t off = 0;
	while((len = read(fd, buf, BUFFER_SIZE)) > 0){
		snprintf(raw + off, len, "%s", buf);
		off += len;
	}

	raw[off] = '\0';
	workergroup_deserialize(raw, w);

	close(fd);

	return w;
}

// add new worker, add new entry to current_deamon_list
int workergroup_add(WRKGRP* grp, const WORKER* worker){
	list_add(grp->workers, worker);
	
	char buf[BUFFER_SIZE];
	int fd;
	if(workergroup_serialize(grp, buf) != NULL){
		if((fd = open(grp->path, O_RDWR | O_CREAT | O_TRUNC)) < 0)
			return -1;
		lseek(fd, 0, SEEK_SET);
		write(fd, buf, strlen(buf));
		close(fd);
		return list_count(grp->workers);
	}
	
	return -1;
}

// remove worker, it does not kill daemon process, only reloading current_deamon_list
// and list
int workergroup_remove(WRKGRP* grp, const WORKER* worker){
	char buf[BUFFER_SIZE];
	int fd;
	if(workergroup_serialize(grp, buf) != NULL){
		if((fd = open(grp->path, O_RDWR | O_CREAT | O_TRUNC)) < 0)
			return -1;
		lseek(fd, 0, SEEK_SET);
		write(fd, buf, strlen(buf));
		close(fd);
		return list_count(grp->workers);
	}

	return -1;
}

// not used
void workergroup_free(WRKGRP* grp){
	
}


char* workergroup_serialize(const WRKGRP* grp, char* out){
	 ITERATOR* it = list_iterator(grp->workers);
	 int off = 0;
	 while(iterator_has_next(it)){
	 	WORKER* w = (WORKER*) iterator_next(it)->value;
		off += snprintf(out + off, strlen(w->path) + 2, "%s\n", w->path);
	 }

	 free(it);

	 return out;
}

WRKGRP* workergroup_deserialize(const char* raw, WRKGRP* out){
	char* tok;
	char input[BUFFER_SIZE];
	strcpy(input, raw);

	char pathbuf[BUFFER_SIZE];
	int fd;

	char* ptr;
	tok = strtok_r(input, "\n", &ptr);
	while(tok != NULL){
		pathutils_join(tok, CONF_NAME, pathbuf);
		if((fd = fileutils_lock(pathbuf)) < 0){
			tok = strtok_r(NULL, "\n", &ptr);
			continue;
		}
		
		CONF* c = (CONF*) malloc(sizeof(CONF));
		strcpy(c->conf_path, pathbuf);
		if(conf_open(fd, c) == NULL){
			conf_free(c);
			fileutils_unlock(fd);
			close(fd);
			tok = strtok_r(NULL, "\n", &ptr);
			continue;
		}
		
		fileutils_unlock(fd);
		close(fd);

		pathutils_join(tok, LOG_NAME, pathbuf);
		LOG* l = (LOG*) malloc(sizeof(LOG));
		if(logger_open(pathbuf, c->max_log_lines, l) == NULL){
			conf_free(c);
			logger_free(l);
			tok = strtok_r(NULL, "\n", &ptr);
			continue;
		}
		
		WORKER* worker = (WORKER*) malloc(sizeof(WORKER));
		strcpy(worker->path, tok);
		worker->conf = c;
		worker->log = l;
		tok = strtok_r(NULL, "\n", &ptr);

		list_add(out->workers, worker);
	}

	return out;
}
void workergroup_nop(void* n){}
// kill all daemon processes
void workergroup_close(WRKGRP* grp){
	list_free(grp->workers, workergroup_nop);
	free(grp);
}