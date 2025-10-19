#include "api.h"
#include "conf.h"
#include "path_utils.h"
#include "logger.h"
#include "worker.h"
#include "string_utils.h"
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ADD_D 1 << 1
#define ADD_I 1 << 2
#define ADD_L 1 << 3
#define ADD_X 1 << 4
#define ADD_E 1 << 5
#define ADD_M 1 << 6

char* dirpath;
char absolute_path[PATH_MAX];
char output_path[PATH_MAX];
char logger_path[PATH_MAX];

int exclude_path_count = 0;
char exclude_path[10][PATH_MAX] = { 0 };
int extensions_count = 0;
char extensions[10][20];
int time_interval = 10;
int max_log_lines = INT_MAX;
DUP_STRATEGY mode = INCLUDE_LATEST;

int options;

int parse_options(int, char* argv[]);
int validate_path();
int validate_output_path();
int validate_exclude_path();

// print usage
void add_help(int detail){
	if(! detail){
		printf("Usage: add <DIR_PATH> [OPTION]..." H_RT);
	}else{
		printf(H_TAB "> add <DIR_PATH> [OPTION]..." H_RT);
		printf(H_TTAB "<none> : add deamon process monitoring the <DIR_PATH> directory" H_RT);
		printf(H_TTAB "-d <OUTPUT_PATH> : Specify the output directory <OUTPUT_PATH> where <DIR_PATH> will be arranged" H_RT);
		printf(H_TTAB "-i <TIME_INTERVAL> : Set the time interval for the daemon process to monitor in seconds." H_RT);
		printf(H_TTAB "-l <MAX_LOG_LINES> : Set the maximum number of log lines the daemon process will record." H_RT);
		printf(H_TTAB "-x <EXCLUDE_PATH1, EXCLUDE_PATH2, ...> : Exclude all subfiles in the specified directories." H_RT);
		printf(H_TTAB "-e <EXTENSION1, EXTENSION2, ...> : Specify the file extensions to be organized." H_RT);
		printf(H_TTAB "-m <M> : Specify the value for the <M> option." H_RT);
	}
}

static char* a_filtername;

// filter worker by path
// used to detect duplication of path
int a_filter_wrk(void* e){
	if(a_filtername == NULL)
		return false;
	WORKER* wrk = (WORKER*) e;
	return stringutils_startwith(wrk->path, a_filtername) || stringutils_startwith(a_filtername, wrk->path);
}

int api_add(int argc, char** argv){
	optind = 0;
	if(argc < 2){
		add_help(false);
		return -1;
	}

	dirpath = argv[1];

	if(validate_path() < 0){
		return -1;
	}
	
	a_filtername = absolute_path;
	if(list_find_any(wrkgrp->workers, a_filter_wrk) != NULL){
		return -1;
	}
	a_filtername = NULL;

	sprintf(output_path, "%s_arranged", absolute_path);

	if(parse_options(argc, argv) < 0){
		return -1;
	}

	char cpath[PATH_MAX];
	pathutils_join(absolute_path, CONF_NAME, cpath);
	CONF* conf = (CONF*) malloc(sizeof(CONF));
	int cfd;
	if((cfd = open(cpath, O_RDWR | O_CREAT, S_IRWXU)) < 0)
		return -1;
	conf_open(cfd, conf);
	close(cfd);
	
	strcpy(conf->monitoring_path, absolute_path);



	if(options & ADD_D){
		if(validate_output_path() < 0){
			conf_free(conf);
			return -1;
		}
	}
	
	strcpy(conf->output_path, output_path);
	
	if(options & ADD_E){
		for(int i = 0; i < extensions_count; i++){
			strcpy(conf->extension[i], extensions[i]);
		}

		conf->extension_count = extensions_count;
	}

	if(options & ADD_X){
		if(validate_exclude_path() < 0){
			conf_free(conf);
			return -1;
		}
		for(int i = 0; i < exclude_path_count; i++){
			strcpy(conf->exclude_path[i], exclude_path[i]);
		}
		conf->exclude_path_count = exclude_path_count;
	}
	
	conf->mode = mode;
	conf->time_interval = time_interval;
	conf->max_log_lines = max_log_lines;
	conf->start_time = time(NULL);
	strcpy(conf->conf_path, cpath);
	if(conf_flush(conf, cpath) < 0)
		return -1;

	pathutils_join(absolute_path, LOG_NAME, logger_path);
	LOG* log = (LOG*) malloc(sizeof(LOG));
	
	if(logger_open(logger_path, max_log_lines, log) == NULL)
		return -1;
	
	WORKER* w = (WORKER*) malloc(sizeof(WORKER));
	strcpy(w->path, absolute_path);
	w->conf = conf;
	w->log = log;

	workergroup_add(wrkgrp, w);
	if(worker_start(w) < 0)
		return -1;

	return 0;
}

// check dir path is valid on condition
int validate_path(){
	pathutils_real_path(dirpath, absolute_path);

	int vflg = pathutils_validate(absolute_path);

	if(vflg & VAL_NOTEXIST)
		return -1;
	if(vflg & VAL_NONDIR)
		return -1;
	if(vflg & VAL_OUTHOME){
			printf("%s is outside the home directory\n", dirpath);
			return -1;
	}
	if(vflg & VAL_IACCESS)
		return -1;


	return 0;
}

// check output_path is valid on condition
int validate_output_path(){
	char out[PATH_MAX];
	pathutils_real_path(output_path, out);
	int vflg = pathutils_validate(out);
	
	if(vflg & VAL_NOTEXIST)
		return -1;
	if(vflg & VAL_NONDIR)
		return -1;
	if(vflg & VAL_OUTHOME)
		return -1;
	if(vflg & VAL_IACCESS)
		return -1;
	if(stringutils_startwith(out, absolute_path))
		return -1;

	strcpy(output_path, out);
	return 0;
}

// check exclude_path is valid on condition
int validate_exclude_path(){
	char abs_path[PATH_MAX];
	char hist[10][PATH_MAX];
	for(int i = 0; i < exclude_path_count; i++){
		pathutils_join(absolute_path, exclude_path[i], abs_path);
		strcpy(hist[i], abs_path);
		int vflg = pathutils_validate(abs_path);
		if(vflg & VAL_NOTEXIST || vflg & VAL_NONDIR || vflg & VAL_OUTHOME || vflg & VAL_IACCESS)
			return -1;
		if(! stringutils_startwith(abs_path, absolute_path))
			return -1;

		for(int j = 0; j < i; j++){
			if(stringutils_strstartwith(hist[j], abs_path) || stringutils_startwith(abs_path, hist[j]))
				return -1;
		}
		strcpy(exclude_path[i], abs_path);	
	}

	return 0;
}

// parse options
int parse_options(int argc, char* argv[]){
	char **tmp;
	int opt, x = 0, e = 0;

	while((opt = getopt(argc, argv, "dxeilm")) != -1){
		switch(opt){
			case 'd':
				options |= ADD_D;
				sprintf(output_path, "%s", argv[optind]);
				break;
			case 'x':
				options |= ADD_X;
				char* ee = argv[optind];
				char* tok = strtok(ee,",");
				while(tok != NULL){
					strcpy(exclude_path[x++], tok);
					tok = strtok(NULL, ",");
				}
				
				exclude_path_count = x;
				break;
			case 'e':
				options |= ADD_E;
				ee = argv[optind];
				tok = strtok(ee, ",");
				while(tok != NULL){
					strcpy(extensions[e++], tok);
					tok = strtok(NULL, ",");
				}
				
				extensions_count = e;
				break;
			case 'i':
				options |= ADD_I;
				time_interval = (int) strtol(argv[optind], NULL, 10);
				if(time_interval < 1)
					return -1;
				if(errno == ERANGE)
					return -1;
				break;
			case 'l':
				options |= ADD_L;
				max_log_lines = (int) strtol(argv[optind], NULL, 10);
				if(max_log_lines < 1)
					return -1;
				if(errno == ERANGE)
					return -1;
				break;
			case 'm':
				options |= ADD_M;
				int m = (int) strtol(argv[optind], NULL, 10);
				if(errno == ERANGE)
					return -1;
				if(! (m >= 1 && m <= 3))
					return -1;
				mode = (DUP_STRATEGY) m;
				break;
			default:
				return -1;
		}
	}

	return 0;
}
