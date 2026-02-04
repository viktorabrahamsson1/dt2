#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "kernel_functions.h"

list *init_list(void);
listobj *create_listobj(TCB *pTask);
listobj *extract(list *li, listobj *obj);
void insert_tail(list *l, listobj *obj);
void insert_sorted(list *l, listobj *obj);


#endif