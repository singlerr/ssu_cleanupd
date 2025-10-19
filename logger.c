#include "logger.h"
#include "file_utils.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#define DEFAULT_SIZE 2048

#define LOG_MAXLEN 10

// open ssu_cleanupd.log file
LOG* logger_open(const char* pathname,int max_log_lines, LOG *out){
	LOG* o;
	if(out == NULL){
		o = (LOG*) malloc(sizeof(LOG));
	}else{
		o = out;
	}

	o->max_log_lines = max_log_lines;
	strcpy(o->path, pathname);
	int fd;
	if((fd = fileutils_lock(pathname)) < 0){
		return NULL;
	}

	fileutils_unlock(fd);
	close(fd);
	return o;
}

// resize log file to fit with new max_log_lines, removing oldest lines first if exceeds
int logger_resize(LOG* handle, int new_cap){
	int fd;

    if ((fd = fileutils_lock(handle->path)) < 0)
        return 0;

    lseek(fd, 0, SEEK_SET);

    char c;
    int count = 0;

    while (read(fd, &c, 1) > 0) {
        if (c == '\n')
            count++;
    }

    if (new_cap >= count) {
        fileutils_unlock(fd);
        close(fd);
        return 0;
    }

    char **lines = malloc(sizeof(char*) * new_cap);
    for (int i = 0; i < new_cap; i++) {
        lines[i] = malloc(sizeof(char) * BUFFER_SIZE);
    }

    int skip = count - new_cap;
    int current_line = 0;
    int storing = 0;
    int i = 0;
    char sbuf[BUFFER_SIZE];

    lseek(fd, 0, SEEK_SET);
    while (read(fd, &c, 1) > 0) {
        if (i < BUFFER_SIZE - 1) {
            sbuf[i++] = c;
        }

        if (c == '\n') {
            sbuf[i] = '\0';
            if (current_line >= skip && storing < new_cap) {
                strncpy(lines[storing], sbuf, BUFFER_SIZE - 1);
                lines[storing][BUFFER_SIZE - 1] = '\0';
                storing++;
            }
            i = 0;
            current_line++;
        }
    }

    if (i > 0 && current_line >= skip && storing < new_cap) {
        sbuf[i] = '\0';
        strncpy(lines[storing], sbuf, BUFFER_SIZE - 1);
        lines[storing][BUFFER_SIZE - 1] = '\0';
        storing++;
    }

    fileutils_unlock(fd);
    close(fd);


    truncate(handle->path, 0);

    fd = fileutils_lock(handle->path);
    if (fd < 0) {
        for (int i = 0; i < new_cap; i++) free(lines[i]);
        free(lines);
        return -1;
    }

    for (int i = 0; i < storing; i++) {
        size_t len = strlen(lines[i]);
        if (len > 0 && lines[i][len - 1] != '\n') {
            strcat(lines[i], "\n");
            len++;
        }

        if (write(fd, lines[i], len) != len) {
            fileutils_unlock(fd);
            close(fd);
            for (int j = 0; j < new_cap; j++) free(lines[j]);
            free(lines);
            return -1;
        }
    }

    fileutils_unlock(fd);
    close(fd);

    for (int i = 0; i < new_cap; i++) free(lines[i]);
    free(lines);

    return new_cap;
}

// append new log line to file, oldest line is removed if exceeds
int logger_append(LOG* handle, const char* line){
	int fd = fileutils_lock(handle->path);
    if (fd < 0) return 0;

    char c;
    char sbuf[BUFFER_SIZE];
    int count = 0;

    lseek(fd, 0, SEEK_SET);
    while (read(fd, &c, 1) > 0) {
        if (c == '\n') count++;
    }

    int max_lines = handle->max_log_lines;
    int line_count = (count >= max_lines) ? max_lines : count + 1;

    char** lines = malloc(sizeof(char*) * line_count);
    for (int i = 0; i < line_count; i++) {
        lines[i] = malloc(sizeof(char) * BUFFER_SIZE);
    }

    lseek(fd, 0, SEEK_SET);
    int current_line = 0;
    int keep_start = count - (line_count - 1);
    if (keep_start < 0) keep_start = 0;

    int i = 0;
    int storing = 0;
    while (read(fd, &c, 1) > 0) {
        sbuf[i++] = c;
        if (c == '\n') {
            sbuf[i] = '\0';
            if (current_line >= keep_start && storing < line_count - 1) {
                strncpy(lines[storing], sbuf, BUFFER_SIZE - 1);
                storing++;
            }
            current_line++;
            i = 0;
        }
    }

    snprintf(lines[storing], BUFFER_SIZE, "%s", line);

    fileutils_unlock(fd);
    close(fd);
    truncate(handle->path, 0);

    fd = fileutils_lock(handle->path);
    if (fd < 0) {
        for (int j = 0; j < line_count; j++) free(lines[j]);
        free(lines);
        return -1;
    }

    for (int j = 0; j < line_count; j++) {
        size_t len = strlen(lines[j]);
        if (len > 0) {
            if (write(fd, lines[j], len) != len) {
                fileutils_unlock(fd);
                close(fd);
                for (int k = 0; k < line_count; k++) free(lines[k]);
                free(lines);
                return -1;
            }
        }
    }

    fileutils_unlock(fd);
    close(fd);

    for (int j = 0; j < line_count; j++) free(lines[j]);
    free(lines);

    return line_count;
}

// not used
int logger_flush(LOG* handle){
	return 0;
}

// get all lines from log file
int logger_get(LOG* handle, char** out){
	int fd;
    if ((fd = fileutils_lock(handle->path)) < 0)
        return -1;

    lseek(fd, 0, SEEK_SET);

    char sbuf[BUFFER_SIZE];
    int p = 0, i = 0;
    char c;

    while (read(fd, &c, 1) > 0) {
        if (i < BUFFER_SIZE - 1) {
            sbuf[i++] = c;
        }

        if (c == '\n') {
            sbuf[i] = '\0';
            strncpy(out[p], sbuf, BUFFER_SIZE - 1);
            out[p][BUFFER_SIZE - 1] = '\0';
            p++;
            i = 0;
        }
    }


    if (i > 0) {
        sbuf[i] = '\0';
        strncpy(out[p], sbuf, BUFFER_SIZE - 1);
        out[p][BUFFER_SIZE - 1] = '\0';
        p++;
    }

    fileutils_unlock(fd);
    close(fd);
    return p; 
}

int logger_nget(LOG* handle, int size, char out[][BUFFER_SIZE]){
	int fd;
    if ((fd = fileutils_lock(handle->path)) < 0)
        return -1;

    char sbuf[BUFFER_SIZE];
    char c;

    int count = 0;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &c, 1) > 0) {
        if (c == '\n') count++;
    }

    int start = count - size;
    if (start < 0) start = 0;

    lseek(fd, 0, SEEK_SET);

    int current_line = 0;
    int i = 0;
    int stored = 0;

    while (read(fd, &c, 1) > 0) {
        if (i < BUFFER_SIZE - 1) {
            sbuf[i++] = c;
        }

        if (c == '\n') {
            sbuf[i] = '\0';
            if (current_line >= start && stored < size) {
                size_t len = strlen(sbuf);
                if (len > 0 && sbuf[len - 1] == '\n') {
                    sbuf[len - 1] = '\0';
                }
                strncpy(out[stored], sbuf, BUFFER_SIZE - 1);
                out[stored][BUFFER_SIZE - 1] = '\0';
                stored++;
            }
            i = 0;
            current_line++;
        }
    }

    if (i > 0 && current_line >= start && stored < size) {
        sbuf[i] = '\0';
        strncpy(out[stored], sbuf, BUFFER_SIZE - 1);
        out[stored][BUFFER_SIZE - 1] = '\0';
        stored++;
    }

    fileutils_unlock(fd);
    close(fd);

    return stored;
}

void logger_free(LOG* log){
	free(log);
}
