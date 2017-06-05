/*********************************************************************************
*FileName: printtree.c
*Author: Debbie Xie
*Create Time: 2017/5/30
*Description:
**********************************************************************************/

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "printtree.h"

/* local function prototype */
static void PT_tree_exp(FILE *out, TR_exp expression, int d);

static void indent(FILE *out, int de) {
 int index;
 for (index = 0; index <= de; index++) fprintf(out, " | ");
}

static char bin_opera[][12] = {
   "PLUS", "MINUS", "TIMES", "DIVIDE", 
   "AND", "OR", "LSHIFT", "RSHIFT", "ARSHIFT", "XOR"};

static char rel_opera[][12] = {
  "EQ", "NE", "LT", "GT", "LE", "GE", "ULT", "ULE", "UGT", "UGE"};
 
static void PT_stm(FILE *outFile, TR_stm statement, int de)
{
  switch (statement->kind) {
  case TR_SEQ:
    indent(outFile,de);
    fprintf(outFile, "SEQ\n"); PT_stm(outFile, statement->u.SEQ.left,de+1);  fprintf(outFile, "\n"); 
    PT_stm(outFile, statement->u.SEQ.right,de+1);
    break;
  case TR_LABEL:
    indent(outFile,de); fprintf(outFile, "LABEL %s", SB_name(statement->u.LABEL));
    break;
  case TR_JUMP:
    indent(outFile,de); fprintf(outFile, "JUMP\n"); PT_tree_exp(outFile, statement->u.JUMP.exp,de+1); 
    break;
  case TR_CJUMP:
    indent(outFile,de); 
	fprintf(outFile, "CJUMP\n");
	indent(outFile, de+1);
	fprintf(outFile, "%s\n", rel_opera[statement->u.CJUMP.op]);
    PT_tree_exp(outFile, statement->u.CJUMP.left,de+1); fprintf(outFile, "\n"); 
    PT_tree_exp(outFile, statement->u.CJUMP.right,de+1); fprintf(outFile, "\n");
    indent(outFile,de+1); 
	fprintf(outFile, "%s\n", SB_name(statement->u.CJUMP.true));
	indent(outFile, de + 1);
    fprintf(outFile, "%s", SB_name(statement->u.CJUMP.false));
    break;
  case TR_MOVE:
    indent(outFile,de); fprintf(outFile, "MOVE\n"); PT_tree_exp(outFile, statement->u.MOVE.dst,de+1); 
    fprintf(outFile, "\n");
    PT_tree_exp(outFile, statement->u.MOVE.src,de+1); 
    break;
  case TR_EXP:
    indent(outFile,de); fprintf(outFile, "EXP\n"); PT_tree_exp(outFile, statement->u.EXP,de+1); 
    break;
  }
}

static void PT_tree_exp(FILE *outFile, TR_exp expression, int de)
{
  switch (expression->kind) {
  case TR_BINOP:
    indent(outFile,de); 
	fprintf(outFile, "BINOP\n"); 
	indent(outFile, de+1);
	fprintf(outFile, "%s\n", bin_opera[expression->u.BINOP.op]);
    PT_tree_exp(outFile, expression->u.BINOP.left,de+1); 
	fprintf(outFile, "\n");
    PT_tree_exp(outFile, expression->u.BINOP.right,de+1); 
    break;
  case TR_MEM:
    indent(outFile,de); fprintf(outFile, "MEM");
    fprintf(outFile, "\n"); PT_tree_exp(outFile, expression->u.MEM,de+1);
    break;
  case TR_TEMP:
    indent(outFile,de); fprintf(outFile, "TEMP t%s", 
			   TMP_look(TMP_name(), expression->u.TEMP));
    break;
  case TR_ESEQ:
    indent(outFile,de); fprintf(outFile, "ESEQ\n"); PT_stm(outFile, expression->u.ESEQ.stm,de+1); 
    fprintf(outFile, "\n");
    PT_tree_exp(outFile, expression->u.ESEQ.exp,de+1);
    break;
  case TR_NAME:
    indent(outFile,de); fprintf(outFile, "NAME %s", SB_name(expression->u.NAME));
    break;
  case TR_CONST:
    indent(outFile,de); fprintf(outFile, "CONST %d", expression->u.CONST);
    break;
  case TR_CALL:
    {TR_expList args = expression->u.CALL.args;
     indent(outFile,de); fprintf(outFile, "CALL\n"); PT_tree_exp(outFile, expression->u.CALL.fun,de+1);
     for (;args; args=args->tail) {
       fprintf(outFile, ",\n"); PT_tree_exp(outFile, args->head,de+2);
     }
     fprintf(outFile, ")");
     break;
   }
  } /* end of switch */
}

void printStmList (FILE *outFile, TR_stmList statementList) 
{
  for (; statementList; statementList=statementList->tail) {
    PT_stm(outFile, statementList->head,0); fprintf(outFile, "\n");
  }
}
