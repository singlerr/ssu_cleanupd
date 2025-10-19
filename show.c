#include "api.h"
#include "conf.h"
#include "path_utils.h"
#include "logger.h"
#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define SPACE "\n\n"
#define LOG_MAXLEN 10
#define BUFFER_SIZE 4096

// print usage
void show_help(int detail){
	if(detail){
		printf(H_TAB "> show" H_RT);
		printf(H_TTAB "<none> : show monitoring deamon process info" H_RT);
	}else{
		printf(H_TAB "> show" H_RT);
	}
}

// print target worker info
int print_detail(const WORKER*);

int api_show(int argc, char** argv){
	printf("Current working daemon process list\n");
	
	while(true){
		printf(SPACE);

		printf("0. exit\n");
		int c;
		if(wrkgrp != NULL)
			c = list_count(wrkgrp->workers);
		else
			c = 0;
		for(int i = 0; i < c; i++){
			WORKER* w = list_get(wrkgrp->workers, i);
			printf("%d. %s\n", i + 1, w->path);
		}

		printf(SPACE);
		printf("Select one to see process info : ");
	
		int i;
		if(scanf("%d", &i) != 1 || ! (i >= 0 && i <= c)){
			printf("Please check your input is valid\n");
			while (fgetc(stdin) != '\n')
			{
				
			}
			
			continue;
		}
	
		if(i == 0){
			return 0;
		}


		WORKER *w = list_get(wrkgrp->workers, i - 1);
		if(w == NULL)
			continue;

		print_detail(w);
		break;
	}

	return 0;
}

int print_detail(const WORKER* worker){
	printf("1. config detail\n");
	printf(SPACE);

	char sb[BUFFER_SIZE];
	conf_serialize(worker->conf, sb);
	printf("%s\n", sb);
	printf(SPACE);

	printf("2. log detail\n");
	printf(SPACE);
	
	char logs[LOG_MAXLEN][BUFFER_SIZE];
	int size;
	if((size = logger_nget(worker->log, LOG_MAXLEN,logs)) < 0)
		return -1;

	for(int i = 0; i < size; i++){
		printf("%s\n", logs[i]);
	}

	printf(SPACE);

	return 0;
}

