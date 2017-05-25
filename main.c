/*************************************************************************
	> Created Time: 2017年05月25日 星期四 11时37分20秒
 ************************************************************************/

#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"

extern int yyparse(void);
extern AST_exp absyn_root;

/*
 * parse source code
 */
AST_exp parseFile(string filename){
	ERRMSG_reset(filename);
	if(yyparse() == 0){
		return absyn_root;
	}
	else{
		return NULL;
	}
}

int main(int argc, char **argv){
	AST_exp exp;
	if(argc != 2){
		fprintf(stderr, "you must input filename!");
		exit(1);
	}
	exp = parseFile(argv[1]);
    #ifdef PRINT
	print_absyn_exp(exp, 0);
	printf("\n");
	#endif

	SM_transProgram(exp);
	return 0;
}

