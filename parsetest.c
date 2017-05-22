#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "prabsyn.h"
#define DISPLAY
extern int yyparse(void);
extern AST_exp absyn_root;

/* parse source file fname; 
   return abstract syntax data structure */
AST_exp parse(string fname)
{ERRMSG_reset(fname);
 if (yyparse() == 0) /* parsing worked */
   return absyn_root;
 else return NULL;
}



int main(int argc, char **argv) {
 AST_exp exp; 
 FILE *fp;
 if((fp=fopen("test.s","wt+"))==NULL){
    printf("Cannot open file strike any key exit!");
    exit(1);
 }
 if (argc!=2) {fprintf(stderr,"usage: a.out filename\n"); exit(1);}
 exp = parse(argv[1]);  
#ifdef DISPLAY  
 print_absyn_exp(exp, 0);
 pr_exp(fp, exp, 0);
 printf("\n");
#endif

 //SEM_transProg(exp); 

 return 0;
}
