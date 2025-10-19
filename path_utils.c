#include "path_utils.h"
#include "string_utils.h"
#include "list.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>
#include <pwd.h>
#define INCLUDE_HOME_DIR

#ifdef INCLUDE_HOME_DIR

#include <pwd.h>
#define HOME_VAR_NAME "$HOME"
#define HOME_REL "~"
#endif

#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

// check pathname is dir
int is_dir(const char* pathname){
	struct stat statbuf;
	if(stat(pathname, &statbuf) == -1){
		return false;
	}

	return S_ISDIR(statbuf.st_mode);
}

// replace home variable in string and make it absolute path
char* pathutils_real_path(const char* pathname, char* out){
	char path[PATH_MAX];
	memset(path, 0, sizeof(path));
	char buffer[PATH_MAX];
	strcpy(path, pathname);

#ifdef INCLUDE_HOME_DIR
	struct passwd* pw = getpwuid(getuid());
	stringutils_replace(path, HOME_VAR_NAME, pw->pw_dir, buffer);
	strcpy(path, buffer);

	stringutils_replace(path, HOME_REL, pw->pw_dir, buffer);
	strcpy(path, buffer);
#endif

	char* rt_path = realpath(path, out);
	return rt_path;
}

// join two path into one
char* pathutils_join(const char* a, const char* b, char* out){
	size_t ai = strlen(a);
	size_t bi = strlen(b);
	if(a[ai] == '/'){
		sprintf(out, "%s%s", a, b);
	}else{
		sprintf(out, "%s/%s", a, b);
	}

	return out;
}

// get extension from path
char* pathutils_get_extension(const char* pathname, char* result){
	char *ext = strrchr(pathname, '.');
	if(! ext)
		return NULL;
	strcpy(result, ext + 1);
	return ext;
}

void nop(void* e){}

// copy file
int pathutils_copy(const char* src, const char* dst){
	char buffer[BUFFER_SIZE];
	struct stat statbuf;
	if(stat(src, &statbuf) == -1){
		return -1;
	}

	int srcfd;
	int dstfd;
	if((srcfd = open(src, O_RDONLY)) < 0)
		return srcfd;
	if((dstfd = open(dst, O_CREAT | O_TRUNC | O_RDWR, statbuf.st_mode)) < 0)
		return dstfd;
	
	lseek(srcfd, 0, SEEK_SET);
	int len;
	while((len = read(srcfd, buffer, sizeof(buffer))) > 0){
		write(dstfd, buffer, len);
	}

	close(srcfd);
	close(dstfd);

	return 0;
}	

// create directory
int pathutils_mkdir(const char* path){
	if(mkdir(path, S_IRWXU) == -1){
		if(errno != EEXIST)
			return -1;
	}

	return 0;
}

// create directory recursively, including parents
int pathutils_mkdir_r(const char* path){
	char buffer[PATH_MAX];
	strcpy(buffer, path);
	for(char* p = strchr(buffer + 1, '/'); p; p = strchr(p + 1, '/')){
		*p = '\0';
		if(mkdir(buffer, 0755) == -1){
			if(errno != EEXIST){
				*p = '/';
				return -1;
			}
		}
		*p = '/';
	}

	return 0;
}

// free dirent entries
void pathutils_free_dirent(struct dirent** d, int c){
	for(int i = 0; i < c; i++){
		free(d[i]);
	}
	free(d);
}

// check file exists
int pathutils_file_exists(const char* path){
	struct stat statbuf;
	return stat(path, &statbuf) >= 0;
}

// check path validation and set flag
int pathutils_validate(const char* path){
	int flag = 0;
	struct passwd* pw = getpwuid(getuid());
	
	if(! stringutils_startwith(path, pw->pw_dir)){
		flag |= VAL_OUTHOME;
	}
	
	if(! pathutils_file_exists(path)){
		flag |= VAL_NOTEXIST;
	}

	if(! is_dir(path)){
		flag |= VAL_NONDIR;
	}

	if(! (access(path, R_OK) == 0 && access(path, W_OK) == 0 && access(path, X_OK) == 0)){
		flag |= VAL_IACCESS;
	}

	return flag;
}
