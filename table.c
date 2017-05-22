/*************************************************************************
	> Created Time: 2017年05月22日 星期一 11时26分27秒
 ************************************************************************/

#include <stdio.h>
#include "table.h"
#include "util.h"

static binditem BindItem(void *key, void *value, binditem next, void *previtem){
	binditem ptr = checked_malloc(sizeof(*ptr));
	ptr->key = key;
	ptr->value = value;
	ptr->next = next;
	ptr->previtem = previtem;
}

