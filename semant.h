/*************************************************************************
	> Created Time: 2017年05月22日 星期一 20时04分48秒
 ************************************************************************/

#ifndef _SEMANT_H_
#define _SEMANT_H_

#include "frame.h"
#include "mipsframe.h"
#include "translate.h"
#include "types.h"
#include "util.h"

bool hasError;
/*
 * the main interface of this module
 * if any error has occured, the external hasError will be set TRUE
 */
FRM_fragList SM_transProgram(AST_exp exp);

/*
 * Environment Module
 */
typedef struct EV_item_ *EV_item;

struct EV_item_ {
	enum {
		EV_varItem,
		EV_funItem
	} kind;
	union{
		struct {
			TL_access access;
			TP_tp tp;
		} var;
		struct
		{
			TL_level level;
			TMP_label label;
			TP_tpList formalTypes;
			TP_tp resultType;
		} fun;
	} u;
};

EV_item EV_VarItem(TL_access access, TP_tp tp);
EV_item EV_FunItem(TL_level level, TMP_label label, TP_tpList formalTypes, TP_tp resultType);

SB_table EV_base_typeEv();
SB_table EV_base_valueEv();

/* 
 * Type checking module
 */

struct exp_tp{
	TL_exp exp;
	TP_tp tp;
};

struct exp_tp Exp_Tp(TL_exp exp, TP_tp tp);
struct exp_tp transVar(TL_level level, SB_table valueEV, SB_table typeEV, AST_var v);
struct exp_tp transExp(TL_level level, SB_table valueEV, SB_table typeEV, AST_exp e);
TL_exp transDec(TL_level level, SB_table valueEV, SB_table typeEV, AST_dec d);

#endif
