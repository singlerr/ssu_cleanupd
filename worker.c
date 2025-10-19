#include "worker.h"
#include "list.h"
#include "path_utils.h"
#include "file_utils.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>
#include <libgen.h>
#include <time.h>
#include <malloc.h>

// macro for freeing struct dirent**
#define FREE_DIRENT(entries, count) \
	do { \
		for(int i = 0; i < count; i++){ \
			free(entries[i]); \
		} \
		free(entries); \
	} while(0);


// select path between two conflicting paths with same filename regard to mode
char* select_p(DUP_STRATEGY, const char*, const char*);

// file entry
typedef struct {
	char path[PATH_MAX];
} ent;

// group of entries with same extension
typedef struct {
	char name[PATH_MAX];
	LIST* entries;
} extent;

// append log line to log file, when copy operation executed
int log_worker(LOG* log, int pid, char* src, char* dst){
	time_t t; 
	struct tm* tt;
	char ts[256];
	char buf[1024]; 
	if((t = time(NULL)) > 0 && (tt = gmtime(&t)) != NULL){
		strftime(ts, sizeof(ts), "%T", tt); 
		sprintf(buf, "[%s] [%d] [%s] [%s]\n", ts, pid, src, dst); 
		logger_append(log, buf); 
	} 
}

// ignore . and .. on scandir
int ignore_rel(const struct dirent* entry){
	if(entry == NULL)
		return false;
	return strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0;
}

// free struct
void free_ent(void* e){
	ent* a = (ent*) e;
	if(a != NULL)
		free(a);
}

// free struct and its list
void free_extent(void* e){
	extent* ee = (extent*) e;
	list_free(ee->entries, &free_ent);
	free(ee->entries);
	free(e);
}

static char* w_filter_name = NULL;

// filter entry with extension
int filter_ext(void* e){
	extent* ext = (extent*) e;
	if(w_filter_name == NULL)
		return true;
	return strcmp(ext->name, w_filter_name) == 0;
}

// filter entry with filename
int filter_filename(void* e){
	if(w_filter_name == NULL)
		return false;
	ent* _ent = (ent*) e;
	char buf[PATH_MAX];
	strcpy(buf, _ent->path);
	char* filename = basename(buf);
	
	return strcmp(filename, w_filter_name) == 0;
}

char* select_p(DUP_STRATEGY mode, const char* a, const char* b){
	struct stat astbuf;
	struct stat bstbuf;

	if(stat(a, &astbuf) == -1 || stat(b, &bstbuf) == -1)
		return NULL;
	
	switch(mode){
		case INCLUDE_LATEST:
			if(astbuf.st_mtime < bstbuf.st_mtime)
				return b;
			else if(astbuf.st_mtime > bstbuf.st_mtime)
				return a;
			else
				return NULL;
		case INCLUDE_OLDEST:
			if(astbuf.st_mtime > bstbuf.st_mtime)
				return b;
			else if(astbuf.st_mtime < bstbuf.st_mtime)
				return a;
			return NULL;
		case EXCLUDE:
			return NULL;
		default:
			return NULL;
			
	}

	return NULL;
}

// copy selected path between conflicting paths regard to current mode
int resolve_conflicts(const WORKER* worker, const char* a, const char* b, const char* out){
	char* selected = select_p(worker->conf->mode, a, b);
	if(selected == NULL)
		return -1;	

	return pathutils_copy(selected, out);
}

// create struct
ent* ent_create(char* path){
	ent* e = (ent*) malloc(sizeof(ent));
	strcpy(e->path, path);
	return e;
}

// create struct
extent* extent_create(char* name){
	extent* e = (extent*) malloc(sizeof(extent));
	strcpy(e->name, name);
	e->entries = list_create();
	return e;
}

// add to queue to scan deeper path entries if input path is directory
// or get extension from path and group it to one list, as long as its extension and path is valid with provided exclude_path and extension
ent* add_or_queue(const WORKER* worker, struct dirent* dent, char* parent_path, LIST* pathqueue, LIST* ext_list){
	char* full_path = (char*) malloc(sizeof(char) * PATH_MAX);
	if(pathutils_join(parent_path, dent->d_name, full_path) == NULL){
		free(full_path);
		return NULL;
	}

	if(strcmp(dent->d_name, CONF_NAME) == 0){
		free(full_path);
		return NULL;
	}

	if(strcmp(dent->d_name, LOG_NAME) == 0){
		free(full_path);
		return NULL;
	}

	struct stat statbuf;
	if(stat(full_path, &statbuf) == -1){
		free(full_path);
		return NULL;
	}
	
	if(S_ISDIR(statbuf.st_mode)){
		for(int i = 0; i < worker->conf->exclude_path_count; i++){
			if(stringutils_strstartwith(full_path, worker->conf->exclude_path[i])){
				free(full_path);
				return NULL;
			}
		}

		list_add(pathqueue, full_path);
	    return NULL;	
	}

	char extname[PATH_MAX];
	char* r = pathutils_get_extension(dent->d_name, extname);
	if(r == NULL){
		extname[0] = '\0';
	}

	// not allowed if not included in provided extension list
	if(worker->conf->extension_count > 0){
		int flg = false;
		for(int i = 0; i < worker->conf->extension_count; i++){
			if(stringutils_strequal(worker->conf->extension[i], extname)){
				flg = true;
				break;
			}
		}

		if(! flg){
			free(full_path);
			return NULL;
		}
	}
	
	w_filter_name = extname;
	extent* ext = list_find_any(ext_list, &filter_ext);
	w_filter_name = NULL;
	int add = false;
	if(ext == NULL){
		ext = extent_create(extname);
		list_add(ext_list, ext);
	}
	w_filter_name = dent->d_name;
	NODE* dup = list_find_node(ext->entries, &filter_filename);
	w_filter_name = NULL;
	if(dup != NULL){
		ent *dup_ent = (ent*) (dup->value);
		char* selected = select_p(worker->conf->mode, dup_ent->path, full_path);
		if(selected != full_path){
			free(full_path);
			return dup_ent;
		}else{
			ent* o_ = (ent*) list_remove(ext->entries, dup);
			free(o_);
		}
	}

	ent* entry = ent_create(full_path);	
	list_add(ext->entries, entry);
	free(full_path);
	return entry;
}

// first scan entries and combine result into list
// bootstrap of scan_entries
void _scan_entries(const WORKER* worker, LIST* ext_entries, LIST* pathqueue, const char* root_path){
	struct dirent **d_list;
	int ecount = scandir(root_path, &d_list, ignore_rel, alphasort);

	for(int i = 0; i < ecount; i++){
		struct dirent* d_ent = d_list[i];
		add_or_queue(worker, d_ent, root_path, pathqueue, ext_entries);
	}

	FREE_DIRENT(d_list, ecount);
}

// deep scan entries and combine result into list
LIST* scan_entries(const WORKER* worker, const char* root_path){
	LIST* ext_entries = list_create();
	LIST* pathqueue = list_create();
	_scan_entries(worker, ext_entries, pathqueue, root_path);

	struct dirent **d_list;
	char* p = NULL;
	while((p = (char*) list_remove_first(pathqueue)) != NULL){
		int ecount = scandir(p, &d_list, &ignore_rel, alphasort);
		for(int i = 0; i < ecount; i++){
			struct dirent* d_ent = d_list[i];
			add_or_queue(worker, d_ent, p, pathqueue, ext_entries);
		}

		FREE_DIRENT(d_list, ecount);
	}
	
	free(pathqueue);
	return ext_entries;
}

// copy entries scanned with scan_entries and log copy operation into log file
void copy_entries(const WORKER* worker, LIST* entries, const char* dst){
	ITERATOR* it = list_iterator(entries);

	char entname[PATH_MAX];
	char dstbuf[PATH_MAX];
	char entbuf[PATH_MAX];
	while(iterator_has_next(it)){
		NODE* n = iterator_next(it);
		extent* eent = (extent*) n->value;
		pathutils_join(dst, eent->name, dstbuf);
		pathutils_mkdir_r(dstbuf);

		ITERATOR* entit = list_iterator(eent->entries);
		while(iterator_has_next(entit)){
			NODE* entn = iterator_next(entit);
			ent* entp = (ent*) entn->value;
			strcpy(entname, entp->path);
			char basebuf[PATH_MAX];
			strcpy(basebuf, entname);
			char* ename = basename(basebuf);
			pathutils_join(dstbuf, ename, entname);
			if(pathutils_file_exists(entname)){
				char* s = select_p(worker->conf->mode, entname, entp->path);
				if(s == entp->path){
					pathutils_copy(entp->path, entname);
					log_worker(worker->log, worker->conf->pid, entp->path, entname);
				}
			}else{
				pathutils_mkdir_r(entname);
				pathutils_copy(entp->path, entname);
				log_worker(worker->log, worker->conf->pid, entp->path, entname);
			}
		}

		free(entit);
	}

	free(it);
}

int loop(WORKER*);
int run(const WORKER* w);
int init_daemon(WORKER* w);

// start daemon process
int worker_start(const WORKER* w){
	int pid;
	if((pid = fork()) < 0){
		exit(1);
	}else if(pid > 0) {
		w->conf->pid = pid;
	}else if(pid == 0){
		int fd = init_daemon(w);
		run(w);
		if(! (fd < 0))
			close(fd);
		exit(0);
	} 
}

// run daemon process's job
// sleep with give time_interval
int run(const WORKER* w){
	int r;
	while(true){
		r = loop(w);
		if(r < 0)
			break;
		sleep(w->conf->time_interval);
	}
}

// reload CONF from file with lock, scan entries, and copy entries
int loop(WORKER* worker){
	if(worker == NULL)
		return -1;
	CONF* conf = worker->conf;
	
	char confp[PATH_MAX];
	pathutils_join(worker->path, CONF_NAME, confp);
	int fd = fileutils_lock(confp);
	if(fd < 0)
		return -1;

	if(conf_open(fd, conf) == NULL)
		return -1;
	
	if(fileutils_unlock(fd) < 0)
		return -1;

	close(fd);

	int p = getpid();
	if(worker->conf->pid != p){
		worker->conf->pid = p;
		if(conf_flush(worker->conf, worker->conf->conf_path) < 0)
			return -1;
	}

	LOG* log = worker->log;
	if(log->max_log_lines != conf->max_log_lines){
		logger_resize(log, conf->max_log_lines);
		log->max_log_lines = conf->max_log_lines;
	}

	char pbuf[PATH_MAX];
	strcpy(pbuf, conf->monitoring_path);
	LIST* src_entries = scan_entries(worker, pbuf);
	strcpy(pbuf, conf->output_path);
	pathutils_mkdir_r(pbuf);
	strcpy(pbuf, conf->output_path);
	copy_entries(worker, src_entries, pbuf);
	list_free(src_entries, free_extent);
	free(src_entries);
	return 0;
}

// free worker
void worker_free(WORKER* worker){
	logger_free(worker->log);
	conf_free(worker->conf);
	free(worker);
}

// setup daemon process
int init_daemon(WORKER* w){
	int pid = getpid();
	
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	setsid();
	int maxfd = getdtablesize();
	int fd;
	for(fd = 0; fd < maxfd; fd++){
		close(fd);
	}
	umask(0);
	fd = open("/dev/null", O_RDWR);
	dup(0);
	dup(0);
	return fd;
}
