/*********************************************************************************
*FileName: assem.h
*Author: Debbie Xie
*Create Time: 2017/6/16
*Description: 
**********************************************************************************/

#ifndef _ASSEM_H_
#define _ASSEM_H_

typedef struct {TMP_labelList labels;} *ASSB_targets;
ASSB_targets ASSB_Targets(TMP_labelList labels);

typedef struct ASSB_instr_ *ASSB_instr;
struct ASSB_instr_ { enum {I_OPER, I_LABEL, I_MOVE} kind;
	       union {struct {string assem; TMP_tempList dst, src; 
			      ASSB_targets jumps;} OPER;
		      struct {string assem; TMP_label label;} LABEL;
		      struct {string assem; TMP_tempList dst, src;} MOVE;
		    } u;
	      };

ASSB_instr ASSB_Oper(string a, TMP_tempList d, TMP_tempList s, ASSB_targets j);
ASSB_instr ASSB_Label(string a, TMP_label label);
ASSB_instr ASSB_Move(string a, TMP_tempList d, TMP_tempList s);

void ASSB_print(FILE *out, ASSB_instr i, TMP_map m);

typedef struct ASSB_instrList_ *ASSB_instrList;
struct ASSB_instrList_ { ASSB_instr head; ASSB_instrList tail;};
ASSB_instrList ASSB_InstrList(ASSB_instr head, ASSB_instrList tail);

ASSB_instrList ASSB_splice(ASSB_instrList a, ASSB_instrList b);
void ASSB_printInstrList (FILE *out, ASSB_instrList iList, TMP_map m);

typedef struct ASSB_proc_ *ASSB_proc;
struct ASSB_proc_ {
  string prolog;
  ASSB_instrList body;
  string epilog;
};

ASSB_proc ASSB_Proc(string p, ASSB_instrList b, string e);

/*Added by Ming Zhou*/
void ASSB_format(char *result, string assem, 
		   TMP_tempList dst, TMP_tempList src,
		   ASSB_targets jumps, TMP_map m);
#endif