#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include <malloc.h>
#include <unistd.h>
// free linked list node
void* free_node(NODE* n){
	if(n == NULL)
		return NULL;
	void* v = n->value;
	if(n->prev != NULL){
		n->prev->next = n->next;
	}
	if(n->next != NULL){
		n->next->prev = n->prev;
	}
	
	free(n);	
	return v;
}

// free linked list and its nodes
void list_free(LIST* handle,  void (*entry_free)(void*)){
	ITERATOR* it = list_iterator(handle);
	while(iterator_has_next(it)){
		NODE* t = iterator_next(it);
		void* v = free_node(t);
		entry_free(v);
	}
	free(it);
}

// create new list
LIST* list_create(){
	LIST* p = (LIST*) malloc(sizeof(LIST));
	p->first = NULL;
	p->last = NULL;
	return p;
}

// check list is empty
int list_is_empty(LIST* handle){
	return handle->first == NULL && handle->last == NULL;
}

// create new node to hold value
NODE* create_node(void* e){
	NODE* n = (NODE*) malloc(sizeof(NODE));
	n->value = e;
	n->prev = NULL;
	n->next = NULL;

	return n;
}

// create iterator of list for iterating entries
ITERATOR* list_iterator(const LIST* handle){
	ITERATOR* it = (ITERATOR*) malloc(sizeof(ITERATOR));
	it->current = handle->first;
	return it;
}

// get entry at given index
void* list_get(const LIST* handle, int index){
	int p = 0;
	NODE* n = handle->first;
	
	while(n != NULL){
		if(p == index){
			return n->value;
		}
		n = n->next;
		p++;
	}

	return NULL;
}

// add new value to list
NODE* list_add(LIST* handle,const void* e){
	if(e == NULL)
		return NULL;
	NODE* n = create_node(e);
	if(list_is_empty(handle)){
		handle->first = n;
		handle->last = n;
		return n;
	}

	handle->last->next = n;
	n->prev = handle->last;
	handle->last = n;
	return n;
}

// get first value of list and remove
void* list_remove_first(LIST* handle){
	if(list_is_empty(handle))
		return NULL;
	
	NODE *o = handle->first;
	if(o->next != NULL){
		handle->first = o->next;
		handle->first->prev = NULL;
	}else{
		handle->first = NULL;
		handle->last = NULL;
	}
	
	return free_node(o);		
}

// get last value of list and remove
void* list_remove_last(LIST* handle){
	if(list_is_empty(handle)){
		return NULL;
	}
	
	NODE *o = handle->last;
	if(o->prev != NULL){
		o->prev->next = NULL;
		handle->last = o->prev;	
	}else{
		handle->first = NULL;
		handle->last = NULL;
	}

	return free_node(o);
}

// find first matching entry
void* list_find_any(const LIST* handle, int (*filter)(void*)){
	if(list_is_empty(handle))
		return NULL;
	ITERATOR* it = list_iterator(handle);
	void* v = NULL;
	while(iterator_has_next(it)){
		NODE* val = iterator_next(it);
		if(filter(val->value)){
			v = val->value;
			break;
		}
	}

	free(it);
	return v;
}

// find first matching node
NODE* list_find_node(const LIST* handle, int (*filter)(void*)){
	ITERATOR* it = list_iterator(handle);
	NODE *n = NULL;
	while(iterator_has_next(it)){
		NODE* node = iterator_next(it);
		if(filter(node->value)){
			n = node;
			break;
		}
	}

	free(it);
	return n;
}

// size of list
size_t list_count(const LIST* handle){
	size_t c = 0;
	if(list_is_empty(handle))
		return c;
	NODE* n = handle->first;
	c++;
	
	while((n = n->next) != NULL){
		c++;
	}

	return c;
}

// check it is safe to call iterater_next
int iterator_has_next(const ITERATOR* handle){
	if(handle->current == NULL)
		return false;
	return true;
}

// get current pointer of iterator
NODE* iterator_next(ITERATOR* handle){
	if(! iterator_has_next(handle))
		return NULL;
	NODE* v = handle->current;
	handle->current = v->next;
	return v;
}

// set value of given node
void* list_set_value(NODE* node, void* data){
	void* o = node->value;
	node->value = data;
	return o;
}

// remove given node from list
void* list_remove(LIST* handle, NODE* n){
	if(list_is_empty(handle)){
		return free_node(n);
	}
	// first check node is head
	if(n->prev == NULL && handle->first == n){
		return list_remove_first(handle);
	}
	if(n->next == NULL && handle->last == n){
		return list_remove_last(handle);
	}
	
	n->prev->next = n->next;
	n->next->prev = n->prev;
	return free_node(n);
}
