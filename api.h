#ifndef _API_H_
#define _API_H_

#define H_TAB "   "
#define H_TTAB "     "
#define H_RT "\n"
#include "worker_group.h"

extern WRKGRP *wrkgrp;

extern void show_help(int);
extern int api_show(int, char**);
extern void add_help(int);
extern int api_add(int, char**);
extern void modify_help(int);
extern int api_modify(int, char**);
extern void remove_help(int);
extern int api_remove(int, char**);
extern void help_help(int);
extern int api_help(int, char**);
extern void exit_help(int);

#endif
