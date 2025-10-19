#include "api.h"
#include "conf.h"
#include "path_utils.h"
#include "logger.h"
#include "worker.h"
#include "file_utils.h"
#include "string_utils.h"
#include <limits.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MOD_D 1 << 1
#define MOD_I 1 << 2
#define MOD_L 1 << 3
#define MOD_X 1 << 4
#define MOD_E 1 << 5
#define MOD_M 1 << 6

char* m_dirpath;
char m_absolute_path[PATH_MAX];
char m_output_path[PATH_MAX];
char m_logger_path[PATH_MAX];

int m_exclude_path_count = 0;
char m_exclude_path[10][PATH_MAX] = { 0 };
int m_extensions_count = 0;
char m_extensions[10][20];
int m_time_interval = 10;
int m_max_log_lines = INT_MAX;
DUP_STRATEGY m_mode = INCLUDE_LATEST;

int m_options;

int parse_m_options(int, char* argv[]);
int m_validate_path();
int validate_m_output_path();
int validate_m_exclude_path();

// print usage
void modify_help(int detail){
	if(! detail){
		printf(H_TAB "> modify <DIR_PATH> [OPTION]..." H_RT);
	}else{
		printf(H_TAB "> modify <DIR_PATH> [OPTION]..." H_RT);
		printf(H_TTAB "<none> : modify deamon process config monitoring the <DIR_PATH> directory" H_RT);
		printf(H_TTAB "-d <OUTPUT_PATH> : Specify the output directory <OUTPUT_PATH> where <DIR_PATH> will be arranged" H_RT);
		printf(H_TTAB "-i <TIME_INTERVAL> : Set the time interval for the daemon process to monitor in seconds." H_RT);
		printf(H_TTAB "-l <MAX_LOG_LINES> : Set the maximum number of log lines the daemon process will record." H_RT);
		printf(H_TTAB "-x <EXCLUDE_PATH1, EXCLUDE_PATH2, ...> : Exclude all subfiles in the specified directories." H_RT);
		printf(H_TTAB "-e <EXTENSION1, EXTENSION2, ...> : Specify the file extensions to be organized." H_RT);
		printf(H_TTAB "-m <M> : Specify the value for the <M> option." H_RT);
	}
}

static char* m_filter_name;

// filter worker with path
int filter_path(void* e){
	if(m_filter_name == NULL)
		return false;

	WORKER* w = (WORKER*) e;
	return strcmp(w->conf->monitoring_path, m_filter_name) == 0;
}

int api_modify(int argc, char** argv){
	optind = 0;

	if(argc < 2){
		modify_help(false);
		return -1;
	}

	m_dirpath = argv[1];

	if(m_validate_path() < 0){
		return -1;
	}
	m_options = 0;
	if(parse_m_options(argc, argv) < 0){
		return -1;
	}

	m_filter_name = m_absolute_path; 
	WORKER *w = (WORKER*) list_find_any(wrkgrp->workers, filter_path);
	m_filter_name = NULL;
	if(w == NULL)
		return -1;
	
	CONF* conf = w->conf;
	if(m_max_log_lines == INT_MAX)
		m_max_log_lines = conf->max_log_lines;

	if(m_options & MOD_D){
		if(validate_m_output_path() < 0){
			return -1;
		}

		if(strcmp(conf->output_path, m_output_path) != 0)
			strcpy(conf->output_path, m_output_path);
	}
	

	
	if(m_options & MOD_E){
		for(int i = 0; i < m_extensions_count; i++){
			strcpy(conf->extension[i], m_extensions[i]);
		}

		conf->extension_count = m_extensions_count;
	}

	if(m_options & MOD_X){
		if(validate_m_exclude_path() < 0){
			return -1;
		}
		for(int i = 0; i < m_exclude_path_count; i++){
			strcpy(conf->exclude_path[i], m_exclude_path[i]);
		}
		conf->exclude_path_count = m_exclude_path_count;
	}
	
	if(m_options & MOD_M)
		conf->mode = m_mode;
	
	if(m_options & MOD_I)
		conf->time_interval = m_time_interval;
	if(m_options & MOD_L)
		conf->max_log_lines = m_max_log_lines;

	if(conf_flush(conf, conf->conf_path) < 0)
		return -1;

	return 0;
}

// check input path is valid on condition
int m_validate_path(){
	pathutils_real_path(m_dirpath, m_absolute_path);

	int vflg = pathutils_validate(m_absolute_path);

	if(vflg & VAL_NOTEXIST)
		return -1;
	if(vflg & VAL_NONDIR)
		return -1;
	if(vflg & VAL_OUTHOME){
		printf("%s is outside the home directory\n", m_dirpath);
		return -1;
	}
	if(vflg & VAL_IACCESS)
		return -1;


	return 0;
}

// check output_path is valid on condition
int validate_m_output_path(){
	char out[PATH_MAX];
	pathutils_real_path(m_output_path, out);
	int vflg = pathutils_validate(out);
	
	if(vflg & VAL_NOTEXIST)
		return -1;
	if(vflg & VAL_NONDIR)
		return -1;
	if(vflg & VAL_OUTHOME)
		return -1;
	if(vflg & VAL_IACCESS)
		return -1;
	if(stringutils_startwith(out, m_absolute_path))
		return -1;

	strcpy(m_output_path, out);
	return 0;
}

// check given exclude_path is valid on condition
int validate_m_exclude_path(){
	char abs_path[PATH_MAX];
	char hist[10][PATH_MAX];
	for(int i = 0; i < m_exclude_path_count; i++){
		pathutils_join(m_absolute_path, m_exclude_path[i], abs_path);
		strcpy(hist[i], abs_path);
		int vflg = pathutils_validate(abs_path);
		if(vflg & VAL_NOTEXIST || vflg & VAL_NONDIR || vflg & VAL_OUTHOME || vflg & VAL_IACCESS)
			return -1;
		if(! stringutils_startwith(abs_path, m_absolute_path))
			return -1;
		for(int j = 0; j < i; j++){
			if(stringutils_strstartwith(hist[j], abs_path) || stringutils_startwith(abs_path, hist[j]))
				return -1;
		}

		strcpy(m_exclude_path[i], abs_path);
	}

	return 0;
}

// parse options
int parse_m_options(int argc, char* argv[]){
	int opt, x = 0, e = 0;
	char* ptr;
	char optionbuf[BUFFER_SIZE];
	while((opt = getopt(argc, argv, "dxeilm")) != -1){
		switch(opt){
			case 'd':
				m_options |= MOD_D;
				sprintf(m_output_path, "%s", argv[optind]);
				break;
			case 'x':
				m_options |= MOD_X;
				strcpy(optionbuf, argv[optind]);
				char* tok = strtok_r(optionbuf,",", &ptr);
				while(tok != NULL){
					strcpy(m_exclude_path[x++], tok);
					tok = strtok_r(NULL, ",", &ptr);
				}
				
				m_exclude_path_count = x;
				break;
			case 'e':
				m_options |= MOD_E;
				strcpy(optionbuf, argv[optind]);
				tok = strtok_r(optionbuf, ",", &ptr);
				while(tok != NULL){
					strcpy(m_extensions[e++], tok);
					tok = strtok_r(NULL, ",", &ptr);
				}
				
				m_extensions_count = e;
				break;
			case 'i':
				m_options |= MOD_I;
				m_time_interval = (int) strtol(argv[optind], NULL, 10);
				if(m_time_interval < 1)
					return -1;
				if(errno == ERANGE)
					return -1;
				break;
			case 'l':
				m_options |= MOD_L;
				m_max_log_lines = (int) strtol(argv[optind], NULL, 10);
				if(m_max_log_lines < 1)
					return -1;
				if(errno == ERANGE)
					return -1;
				break;
			case 'm':
				m_options |= MOD_M;
				int m = (int) strtol(argv[optind], NULL, 10);
				if(errno == ERANGE)
					return -1;
				if(! (m >= 1 && m <= 3))
					return -1;
				m_mode = (DUP_STRATEGY) m;
				break;
			default:
				return -1;
		}
	}

	return 0;
}
