#ifndef _UTIL_H_
#define _UTIL_H_
#include <assert.h>

typedef char *string;
typedef char bool;

#define TRUE 1
#define FALSE 0

void *check_malloc(int);
void check_free(void *ptr);
string String(char *);

typedef struct UN_boolList_ *UN_boolList;
struct UN_boolList_ {bool head; UN_boolList tail;};
UN_boolList UN_BoolList(bool head, UN_boolList tail);



#endif
