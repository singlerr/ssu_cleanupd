#include "conf.h"
#include "string_utils.h"
#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#define BUFFER_SIZE 4096

int deserialize_mode(char* s_mode, DUP_STRATEGY* out){
	if(strlen(s_mode) <= 0)
		return -1;
	int mode = atoi(s_mode);
	*out = mode;
	return mode;
}

size_t serialize_mode(const DUP_STRATEGY mode, char* buffer){
	sprintf(buffer, "mode : %d", (int) mode);
	return strlen(buffer);
}

size_t count_exclude_path(const char* s_p){
	return stringutils_strcnt(s_p, ',');
}

size_t deserialize_exclude_path(const char* s_ep, size_t c, char result[10][PATH_MAX]){
	if(strcmp(s_ep, CONF_NONE) == 0)
		return 0;
	
	size_t buf_len = strlen(s_ep) + 1;
	char* buffer = (char*) malloc(sizeof(char) * buf_len);
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, s_ep);

	size_t idx = 0;
	char* token = strtok(buffer, ",");
	while((token != NULL)){
		strcpy(result[idx++], token);
		token = strtok(NULL, ",");
	}

	free(buffer);
	return idx;
}

size_t serialize_exclude_path(const char exclude_path[10][PATH_MAX], size_t c, char* buffer){
	if(c == 0){
		return sprintf(buffer, "exclude_path : %s", CONF_NONE);
	}

	size_t off = 0;
	int len = c;
	char buf[BUFFER_SIZE];
	for(int i = 0; i < len; i++){
		size_t path_len;
		if(i != len - 1){
			path_len = sprintf(buf + off, "%s,", exclude_path[i]);
		}else{
			path_len = sprintf(buf + off, "%s", exclude_path[i]);
		}

		off += path_len;
	}

	return sprintf(buffer, "exclude_path : %s", buf);
}

size_t count_extension(const char* s){
	return stringutils_strcnt(s, ',') + 1;
}

size_t serialize_extension(const char extension[10][20], size_t c, char* buffer){
	if(c == 0){
		return sprintf(buffer, "extension : %s", CONF_ALL);
	}

	size_t off = 0;
	int len = c;
	char buf[BUFFER_SIZE];
	for(int i = 0; i < len; i++){
		size_t ext_len;
		if(i != len - 1){
			ext_len = sprintf(buf + off, "%s,", extension[i]);
		}else{
			ext_len = sprintf(buf + off, "%s", extension[i]);
		}
		off += ext_len;
	}

	return sprintf(buffer, "extension : %s", buf);
}

size_t deserialize_extension(const char* s_ext, size_t c,  char result[10][20]){
    if(strcmp(s_ext, CONF_ALL) == 0)
		return 0;
	size_t buf_len = strlen(s_ext) + 1;
    char* buffer = (char*) malloc(sizeof(char) * buf_len);
    memset(buffer, 0, buf_len);
    strcpy(buffer, s_ext);

    size_t idx = 0;
    char* token = strtok(buffer, ",");
    while(token != NULL){
        strcpy(result[idx++], token);
        token = strtok(NULL, ",");
	}

	free(buffer);
    return idx;
}

char* deserialize_monitoring_path(const char* s_mp, char* out){
	if(strlen(s_mp) <= 0 || strcmp(s_mp, "none") == 0){
		return NULL;
	}

	strcpy(out, s_mp);
	return out;
}

size_t serialize_monitoring_path(const char* monitoring_path, char* out){
	size_t len = sprintf(out, "monitoring_path : %s", monitoring_path);
	return len;
}

int deserialize_pid(const char* s_pid, int* out){
	if(strlen(s_pid) <= 0)
		return -1;

	int s = atoi(s_pid);
	*out = s;
	return s;
}

size_t serialize_pid(const int pid, char* out){
	int len = sprintf(out, "pid : %d", pid);
	return (size_t)  len;
}

time_t deserialize_start_time(const char* s_st, time_t* out){
	if(strlen(s_st) <= 0){
		return -1;
	}

	struct tm t;
	char* rt = strptime(s_st, CONF_TIME_FORMAT, &t);
	if(rt == NULL)
		return -1;
	
	time_t ft = timegm(&t);
	if(ft < 0)
		return -1;

	*out = ft;

	return ft;
}

size_t serialize_start_time(const time_t start_time, char *out){
	struct tm t;
	gmtime_r(&start_time, &t);

	char buf[BUFFER_SIZE];
	strftime(buf, BUFFER_SIZE, CONF_TIME_FORMAT , &t);
	size_t len = sprintf(out, "start_time : %s", buf);

	return len;
}

char* deserialize_output_path(const char* s_op, char* out){
	if(strlen(s_op) <= 0){
		return NULL;
	}

	strcpy(out, s_op);
	return out;
}

size_t serialize_output_path(const char* output_path, char* out){
	size_t len = sprintf(out, "output_path : %s", output_path);
	return len;
}

int deserialize_time_interval(const char* s_ti, int* out){
	if(strlen(s_ti) <= 0){
		return -1;
	}

	int s = atoi(s_ti);
	*out = s;
	return s;
}

size_t serialize_time_interval(const int time_interval, char* out){
	size_t l; 
	l = sprintf(out, "time_interval : %d", time_interval);
	return l;
}

int deserialize_max_log_lines(const char* s_ml, int* out){
	if(strlen(s_ml) <= 0){
		return -1;
	}
	
	if(strcmp(s_ml, CONF_NONE) == 0){
		*out = INT_MAX;
		return INT_MAX;
	}
	int s = atoi(s_ml);
	*out = s;
	return s;
}

size_t serialize_max_log_lines(const int max_log_lines, char* out){
	size_t l;
	
	if(max_log_lines != INT_MAX){
		l = sprintf(out, "max_log_lines : %d", max_log_lines);
	}else{
		l = sprintf(out, "max_log_lines : none");
	}

	return l;
}

size_t pstrlen(const char* a, const char b){
	size_t i;
	size_t len = strlen(a);
	for(i = 0; i < len && a[i] != b; i++){
		
	}

	return i;
}


// serialize required configuration entries into string for ssu_cleanupd.config
char* conf_serialize(const CONF *conf, char* out){
	char buffer[BUFFER_SIZE];
	char buffer_s[BUFFER_SIZE];
	char *rt_p = &buffer[0];
	size_t off = 0;
	// serialize monitoring_path
	size_t tmp_len = serialize_monitoring_path(conf->monitoring_path, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize pid
	tmp_len = serialize_pid(conf->pid, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize start_time	
	tmp_len = serialize_start_time(conf->start_time, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize output_path
	tmp_len = serialize_output_path(conf->output_path, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize time_interval
	tmp_len = serialize_time_interval(conf->time_interval, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize max_log_lines
	tmp_len = serialize_max_log_lines(conf->max_log_lines, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize exclude_path
	tmp_len = serialize_exclude_path(conf->exclude_path, conf->exclude_path_count, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize extension
	tmp_len = serialize_extension(conf->extension,conf->extension_count, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	// serialize mode
	tmp_len = serialize_mode(conf->mode, buffer_s);
	strncpy(buffer + off, buffer_s, tmp_len);
	off += tmp_len;
	buffer[off++] = '\n';
	buffer[off] = '\0';
	int n = sprintf(out, "%s", buffer);
	out[n] = '\0';
	return out;
}

// read all entries in ssu_cleanupd.config and load it to CONF, to communicate settings file between parent and daemon process
// also for showing info
CONF* conf_deserialize(const char* s_conf, CONF* result){
	size_t e_len;
	char buffer[BUFFER_SIZE];
	char pbuf[BUFFER_SIZE];
	strcpy(pbuf, s_conf);
	char* p = &pbuf[0];
	char* r;
	//deserialize monitoring path
	p = stringutils_consume_or_null(p, "monitoring_path : ");
	NOT_NULL(p)
		
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	r = deserialize_monitoring_path(buffer, result->monitoring_path);	
	NOT_NULL(r)
	//deserialize pid
	p = stringutils_consume_or_null(p, "pid : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	int ri  = deserialize_pid(buffer, &(result->pid));
	NOT_NEG(ri)
	//deserialize start_time
	p = stringutils_consume_or_null(p, "start_time : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	time_t r_t = deserialize_start_time(buffer, &(result->start_time));
	NOT_NEG(r_t)
	//deserialize output_path
	p = stringutils_consume_or_null(p, "output_path : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	r = deserialize_output_path(buffer, result->output_path);
	NOT_NULL(r)
	//deserialize time_inteval
	p = stringutils_consume_or_null(p, "time_interval : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	ri = deserialize_time_interval(buffer, &(result->time_interval));
	NOT_NEG(ri)
	//deserialize max_log_lines
	p = stringutils_consume_or_null(p, "max_log_lines : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	ri = deserialize_max_log_lines(buffer, &(result->max_log_lines));
	NOT_NEG(ri)
	//deserialize exclude_path
	p = stringutils_consume_or_null(p, "exclude_path : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	size_t pcount = count_exclude_path(buffer);
	ri  = deserialize_exclude_path(buffer,pcount,result->exclude_path);
	NOT_NEG(ri)
	result->exclude_path_count = ri;
	//deserialize extension
	p = stringutils_consume_or_null(p, "extension : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	size_t ecount = count_extension(buffer);
	ri  = deserialize_extension(buffer, ecount, result->extension);
	NOT_NEG(ri)
	result->extension_count = ri;
	//deserialize mode
	p = stringutils_consume_or_null(p, "mode : ");
	NOT_NULL(p)
	e_len = pstrlen(p, '\n');
	strncpy(buffer, p, e_len);
	buffer[e_len] = '\0';
	p = &p[e_len + 1];
	ri = deserialize_mode(buffer, &(result->mode));
	NOT_NEG(ri)
	return result;
}

CONF* conf_open(int fd, CONF* out){	
	char in[4096];
	char buf[BUFFER_SIZE];
	lseek(fd, 0, SEEK_SET);
	
	size_t off = 0;
	size_t len;
	while((len = read(fd, buf, sizeof(buf))) > 0){
		snprintf(in + off, len, "%s", buf);
		off += len;
	}
	
	return conf_deserialize(in, out);
}

void conf_free(CONF* conf){
	free(conf);
}

// write serialized config into file
int conf_flush(const CONF* conf, const char* path){
	int fd;
	if((fd = fileutils_lock(path)) < 0)
		return -1;
	ftruncate(fd, 0);

	char buf[BUFFER_SIZE];
	if(conf_serialize(conf, buf) == NULL){
		close(fd);
		return -1;
	}

	lseek(fd, 0, SEEK_SET);
	write(fd, buf, strlen(buf));
	
	fileutils_unlock(fd);
	close(fd);
	return 0;
}
