#include "api.h"
#include "path_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define INCLUDE_HOME_DIR
#define BUFFER_SIZE 1024

WRKGRP* wrkgrp;

// split string by delimiter and create array of strings
char** tokenize_args(char *str, int *argc, char* del){
	*argc = 0;
	char *tmp[100] = { (char*) NULL, };
	char *tok = NULL;
	tok = strtok(str, del);
	while(tok != NULL){
		tmp[(*argc)++] = tok;
		tok = strtok(NULL, del);
	}

	char **argv = (char**) malloc(sizeof(char*) * (*argc + 1));
	for(int i = 0; i < *argc; i++){
		argv[i] = tmp[i];
	}

	return argv;
}

// check give command is available and call command
int handle_cmd(char* all, int argc, char** argv);

// create or load current_deamon_list and initialize daemon processes
int init();

int main(void){
	if(init() < 0)
		exit(0);

	char in[BUFFER_SIZE];
	int argc;
	char **argv = NULL;

	while(true){
		printf("20211430> ");
		fgets(in, BUFFER_SIZE, stdin);
		in[strlen(in) - 1] = '\0';
		if((argv = tokenize_args(in, &argc, " ")) == NULL)
			continue;
		int r = handle_cmd(in, argc, argv);
		free(argv);
		if(r < 0)
			break;
	}
	
	exit(0);
}

// print help usage
void help_help(int detail){
	if(detail){
		printf(H_TAB "> help" H_RT);
	}
}

// print all usages
int api_help(int argc, char** argv){
	printf("Usage:" H_RT);
	show_help(true);
	add_help(true);
	modify_help(true);
	remove_help(true);
	help_help(true);
	exit_help(true);
	return 0;
}

int handle_cmd(char* all, int argc, char** argv){
	if(strcmp(all, "exit") == 0){
		return -1;
	}
	
	if(argc < 1){
		api_help(argc, argv);
		return 0;
	}
		
	if(strcmp(argv[0], "show") == 0){
		api_show(argc, argv);
		while((fgetc(stdin)) != '\n'){}
	}else if(strcmp(argv[0], "add") == 0){
		api_add(argc, argv);
	}else if(strcmp(argv[0], "modify") == 0){
		api_modify(argc, argv);
	} else if(strcmp(argv[0], "remove") == 0){
		api_remove(argc, argv);
	}else{
		api_help(argc, argv);
	}

	return 0;
}



int init(){
	char dpath[BUFFER_SIZE];
	pathutils_real_path("~/.ssu_cleanupd/", dpath);
	if(pathutils_mkdir(dpath) < 0)
		return -1;
	char lpath[BUFFER_SIZE];
	pathutils_join(dpath, "current_deamon_list", lpath);

	if((wrkgrp = workergroup_create(lpath)) == NULL)
		return -1;
}

void exit_help(int detail){
	if(detail){
		printf(H_TAB "> exit" H_RT);
	}
}