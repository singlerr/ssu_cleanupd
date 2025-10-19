#ifndef _LIST_H_
#define _LIST_H_

#include "base.h"
#include <stdio.h>

typedef struct _node{
    struct _node *next;
	struct _node *prev;
	void* value;
} NODE;

typedef struct _list{
	NODE *first;
	NODE *last;	
} LIST;

typedef struct _iterator{
	NODE *current;
} ITERATOR;

extern LIST* list_create PARAMS(());
extern size_t list_count PARAMS((const LIST*));
extern ITERATOR* list_iterator PARAMS((const LIST*));
extern NODE* list_add PARAMS((LIST*,const void*));
extern void* list_get PARAMS((const LIST*, const int));
extern void* list_remove_first PARAMS((LIST*));
extern void* list_remove_last PARAMS((LIST*));
extern int list_is_empty PARAMS((LIST*));
extern int iterator_has_next PARAMS((const ITERATOR*));
extern NODE* iterator_next PARAMS((ITERATOR*));
extern void list_free PARAMS((LIST*, void (*)(void*)));
extern void* list_find_any PARAMS((const LIST*, int (*)(void*)));
extern NODE* list_find_node PARAMS((const LIST*, int (*)(void*)));
extern void* list_set_value PARAMS((NODE*, void*));
extern void* list_remove PARAMS((LIST*, NODE*));
#endif

