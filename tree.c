/*********************************************************************************
*FileName: tree.c
*Author: Debbie Xie
*Create Time: 2017/5/30
*Description: TIGER'S IR Translation Module
**********************************************************************************/

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"

TR_expList TR_ExpList(TR_exp h, TR_expList t)
{TR_expList ptr = (TR_expList) check_malloc (sizeof *ptr);
 ptr->head=h; ptr->tail=t;
 return ptr;
}

TR_stmList TR_StmList(TR_stm h, TR_stmList t)
{TR_stmList ptr = (TR_stmList) check_malloc (sizeof *ptr);
 ptr->head=h; ptr->tail=t;
 return ptr;
}
 
TR_stm TR_Seq(TR_stm l, TR_stm r)
{TR_stm ptr = (TR_stm) check_malloc(sizeof *ptr);
 ptr->kind=TR_SEQ;
 ptr->u.SEQ.left=l;
 ptr->u.SEQ.right=r;
 return ptr;
}

TR_stm TR_Label(TMP_label l)
{TR_stm ptr = (TR_stm) check_malloc(sizeof *ptr);
 ptr->kind=TR_LABEL;
 ptr->u.LABEL=l;
 return ptr;
}
 
TR_stm TR_Jump(TR_exp expression, TMP_labelList lbls)
{TR_stm ptr = (TR_stm) check_malloc(sizeof *ptr);
 ptr->kind=TR_JUMP;
 ptr->u.JUMP.exp=expression;
 ptr->u.JUMP.jumps=lbls;
 return ptr;
}

TR_stm TR_Cjump(TR_relOp opera, TR_exp l, TR_exp r, 
	      TMP_label truel, TMP_label falsel)
{TR_stm ptr = (TR_stm) check_malloc(sizeof *ptr);
 ptr->kind=TR_CJUMP;
 ptr->u.CJUMP.op=opera; ptr->u.CJUMP.left=l; ptr->u.CJUMP.right=r;
 ptr->u.CJUMP.true=truel;
 ptr->u.CJUMP.false=falsel;
 return ptr;
}
 
TR_stm TR_Move(TR_exp destination, TR_exp source)
{TR_stm ptr = (TR_stm) check_malloc(sizeof *ptr);
 ptr->kind=TR_MOVE;
 ptr->u.MOVE.dst=destination;
 ptr->u.MOVE.src=source;
 return ptr;
}
 
TR_stm TR_Exp(TR_exp expression)
{TR_stm ptr = (TR_stm) check_malloc(sizeof *ptr);
 ptr->kind=TR_EXP;
 ptr->u.EXP=expression;
 return ptr;
}
 
TR_exp TR_Binop(TR_binOp opera, TR_exp l, TR_exp r)
{TR_exp ptr = (TR_exp) check_malloc(sizeof *ptr);
 ptr->kind=TR_BINOP;
 ptr->u.BINOP.op=opera;
 ptr->u.BINOP.left=l;
 ptr->u.BINOP.right=r;
 return ptr;
}
 
TR_exp TR_Mem(TR_exp expression)
{TR_exp ptr = (TR_exp) check_malloc(sizeof *ptr);
 ptr->kind=TR_MEM;
 ptr->u.MEM=expression;
 return ptr;
}
 
TR_exp TR_Temp(TMP_temp te)
{TR_exp ptr = (TR_exp) check_malloc(sizeof *ptr);
 ptr->kind=TR_TEMP;
 ptr->u.TEMP=te;
 return ptr;
}
 
TR_exp TR_Eseq(TR_stm statement, TR_exp expression)
{TR_exp ptr = (TR_exp) check_malloc(sizeof *ptr);
 ptr->kind=TR_ESEQ;
 ptr->u.ESEQ.stm=statement;
 ptr->u.ESEQ.exp=expression;
 return ptr;
}
 
TR_exp TR_Name(TMP_label namel)
{TR_exp ptr = (TR_exp) check_malloc(sizeof *ptr);
 ptr->kind=TR_NAME;
 ptr->u.NAME=namel;
 return ptr;
}
 
TR_exp TR_Const(int constint)
{TR_exp ptr = (TR_exp) check_malloc(sizeof *ptr);
 ptr->kind=TR_CONST;
 ptr->u.CONST=constint;
 return ptr;
}
 
TR_exp TR_Call(TR_exp func, TR_expList argus)
{TR_exp ptr = (TR_exp) check_malloc(sizeof *ptr);
 ptr->kind=TR_CALL;
 ptr->u.CALL.fun=func;
 ptr->u.CALL.args=argus;
 return ptr;
}

TR_relOp TR_notRel(TR_relOp re)
{
 switch(re)
   {case TR_eq: return TR_ne;
    case TR_ne: return TR_eq;
    case TR_lt: return TR_ge;
    case TR_ge: return TR_lt;
    case TR_gt: return TR_le;
    case TR_le: return TR_gt;
    case TR_ult: return TR_uge;
    case TR_uge: return TR_ult;
    case TR_ule: return TR_ugt ;
    case TR_ugt: return TR_ule;
  }
 assert(0); return 0;
}

TR_relOp TR_commute(TR_relOp re)
{switch(re) {
    case TR_eq: return TR_eq;
    case TR_ne: return TR_ne;
    case TR_lt: return TR_gt;
    case TR_ge: return TR_le;
    case TR_gt: return TR_lt ;
    case TR_le: return TR_ge;
    case TR_ult: return TR_ugt;
    case TR_uge: return TR_ule;
    case TR_ule: return TR_uge ;
    case TR_ugt: return TR_ult;
   }
 assert(0); return 0;
}
