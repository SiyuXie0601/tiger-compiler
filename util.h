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

#endif
