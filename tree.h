/*********************************************************************************
*FileName: tree.h
*Author: Debbie Xie
*Create Time: 2017/5/30
*Description: TIGER'S IR Translation Module
**********************************************************************************/

#ifndef _TREE_H_
#define _TREE_H

//#include "tree.h"_

typedef struct TR_stm_ *TR_stm;
typedef struct TR_exp_ *TR_exp;
typedef struct TR_expList_ *TR_expList;
struct TR_expList_ {TR_exp head; TR_expList tail;};
typedef struct TR_stmList_ *TR_stmList;
struct TR_stmList_ {TR_stm head; TR_stmList tail;};

typedef enum {TR_plus, TR_minus, TR_mul, TR_div,
	      TR_and, TR_or, TR_lshift, TR_rshift, TR_arshift, TR_xor} TR_binOp ;

typedef enum  {TR_eq, TR_ne, TR_lt, TR_gt, TR_le, TR_ge,
		TR_ult, TR_ule, TR_ugt, TR_uge} TR_relOp;

struct TR_stm_ {enum {TR_SEQ, TR_LABEL, TR_JUMP, TR_CJUMP, TR_MOVE,
		       TR_EXP} kind;
	       union {struct {TR_stm left, right;} SEQ;
		      TMP_label LABEL;
		      struct {TR_exp exp; TMP_labelList jumps;} JUMP;
		      struct {TR_relOp op; TR_exp left, right;
			      TMP_label true, false;} CJUMP;
		      struct {TR_exp dst, src;} MOVE;
		      TR_exp EXP;
		    } u;
	     };

struct TR_exp_ {enum {TR_BINOP, TR_MEM, TR_TEMP, TR_ESEQ, TR_NAME,
		      TR_CONST, TR_CALL} kind;
	      union {struct {TR_binOp op; TR_exp left, right;} BINOP;
		     TR_exp MEM;
		     TMP_temp TEMP;
		     struct {TR_stm stm; TR_exp exp;} ESEQ;
		     TMP_label NAME;
		     int CONST;
		     struct {TR_exp fun; TR_expList args;} CALL;
		   } u;
	    };


TR_expList TR_ExpList(TR_exp h, TR_expList t);
TR_stmList TR_StmList(TR_stm h, TR_stmList t);
TR_stm TR_Seq(TR_stm l, TR_stm r);
TR_stm TR_Label(TMP_label l);
TR_stm TR_Jump(TR_exp expression, TMP_labelList lbls);
TR_stm TR_Cjump(TR_relOp opera, TR_exp l, TR_exp r, 
	      TMP_label truel, TMP_label falsel);
TR_stm TR_Move(TR_exp destination, TR_exp source);
TR_stm TR_Exp(TR_exp expression);
TR_exp TR_Binop(TR_binOp opera, TR_exp l, TR_exp r);
TR_exp TR_Mem(TR_exp expression);
TR_exp TR_Temp(TMP_temp te);
TR_exp TR_Eseq(TR_stm statement, TR_exp expression);
TR_exp TR_Name(TMP_label namel);
TR_exp TR_Const(int constint);
TR_exp TR_Call(TR_exp func, TR_expList argus);

TR_relOp TR_notRel(TR_relOp re);
TR_relOp TR_commute(TR_relOp re);

#endif
