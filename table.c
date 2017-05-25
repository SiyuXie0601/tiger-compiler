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
	return ptr;
}

TB_table TB_create(){
	TB_table newtable = checked_malloc(sizeof(*newtable));
	newtable->curitem = NULL;
	for(int i = 0;i < TBSIZE;i ++){
		newtable->table[i] = NULL;
	}
	return newtable;
}

void TB_push(TB_table t, void *key, void *value){
	int tableidx = 0;
	assert(t && key);
	tableidx = ((unsigned) tableidx) % TBSIZE;
	t->table[tableidx] = BindItem(key, value, t->table[tableidx], t->curitem);
	t->curitem = key;
}

void TB_look(TB_table t, void *key){
	int tableidx = 0;
	assert(t && key);
	binditem item;
	tableidx = ((unsigned) tableidx) % TBSIZE;
	for(item = t->table[tableidx]; item; item = item->next){
		if(item->key == key){
			return item->value;
		}
	}
	return NULL;
}

void TB_pop(TB_table t){
	void *k = NULL;
	int tableidx = 0;
	binditem item;
	k = t->curitem;
	assert(k);
	tableidx = ((unsigned) k) % TBSIZE;
	item = t->table[tableidx];
	assert(item);
	t->table[tableidx] = item->next;
	t->curitem = item->previtem;
	return item->key;
}


