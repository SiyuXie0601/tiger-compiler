/*************************************************************************
	> Created Time: 2017年05月25日 星期四 11时37分20秒
 ************************************************************************/

#include <stdio.h>
#include "util.h"
#include <stdlib.h>
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "semant.h"
#include "frame.h"
#include "assem.h"
#include "canon.h"
#include "tree.h"
#define PRINT
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

static void doProc(FILE *out, FRM_frame frm, TR_stm by){
	static cnt = 0;
	ASSB_instrList instructionList;
	ASSB_proc proc;
	TR_stmList statementList;

	statementList = CA_linearize(by);
	fprintf(out, "\nProcedure %d (%s):\n", ++cnt, TMP_labelstring(FRM_name(frm)));
	instructionList = FRM_codegen(frm, statementList);
	TMP_map map = TMP_layerMap(FRM_tempMap(), TMP_name());
	fprintf(out, "BEGIN %s\n", Temp_labelstring(F_name(frm)));
	AS_printInstrList (out, instructionList, map);
	fprintf(out, "END %s\n", Temp_labelstring(F_name(frm)));
	proc = F_procEntryExit3(frm, instructionList);
}


int main(int argc, char **argv){
	AST_exp exp;
	FRM_fragList frgList;
	FILE *out = stdout;

	if(argc != 2){
		fprintf(stderr, "you must input filename!");
		exit(1);
	}
	exp = parseFile(argv[1]);
    #ifdef PRINT
	print_absyn_exp(exp, 0);
	printf("\n");
	#endif

	frgList = SM_transProgram(exp);
	fprintf(out, "\n(assembly code before register allocation)\n");
	for(; frgList; frgList = frgList->tail){
		if(frgList->head != NULL){
			if(frgList->head->kind == FRM_ProcFrag){
				doProc(out, frgList->head->u.proc.frame, frgList->head->u.proc.body);
			}
		}
	}

	fclose(out);
	system("pause");
	return 0;
}

