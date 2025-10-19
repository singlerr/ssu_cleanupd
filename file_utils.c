#include "file_utils.h"

// open or create file if not exists
FILE* fileutils_open_or_create(const char* pathname){
	FILE* fp;
	fp = NULL;
	if((fp = fopen(pathname, "r+")) == NULL){
		fp = fopen(pathname, "w+");
	}

	return fp;
}

// using fcntl, tries to get lock of file
int fileutils_lock(const char* path){
	int fd;
	if((fd = open(path, O_RDWR | O_CREAT, 0644)) < 0){
		return -1;
	}
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	if(fcntl(fd, F_SETLKW, &lock) == -1){
		close(fd);
		return -1;
	}

	return fd;
}

// tries to unlock file with given file descriptor
int fileutils_unlock(int fd){
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	if(fcntl(fd, F_SETLKW, &lock) == -1){
		close(fd);
		return -1;
	}

	return 0;
}
