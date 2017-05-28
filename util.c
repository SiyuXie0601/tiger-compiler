/*************************************************************************
	> Created Time: 2017年05月22日 星期一 20时13分26秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

void *check_malloc(int length){
	void *ptr = malloc(length);
	if( !ptr ){
		fprintf(stderr, "\nMemory has run out!\n");
		exit(1);
	}
	return ptr;
}

void check_free(void *ptr){
	free(ptr);
}
string String(char *str){
	string ptr = check_malloc(strlen(str)+1);
	strcpy(ptr, str);
	return ptr;
}
