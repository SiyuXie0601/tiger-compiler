/**
 * table.h - hash table
 *
 */

#ifndef _TABLE_H_
#define _TABLE_H_

#define TBSIZE 127
typedef struct binditem_ *binditem;
struct binditem_{
	void *key;
	void *value;
	binditem next;
	void *previtem;
};
typedef struct TB_table_ *TB_table;
struct TB_table_{
    binditem table[TBSIZE];
	void *curitem;
};

/*
 * create a new table mapping "key" to "value"
 */
TB_table TB_create();

/*
 * push the mapping "key" to "value" into table "t"
 * shadowing but not destroying any previous binding for "key" */
void TB_push(TB_table t, void *key, void *value);

/*
 * look up the most recent binding for "key" in table "t"
 */
void *TB_look(TB_table t, void *key);

/*
 * pop the most recent binding and return its key.
 */
void TB_pop(TB_table t);

#endif
