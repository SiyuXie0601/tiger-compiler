/*
 * prabsyn.c - Print Abstract Syntax data structures. Most functions 
 *           handle an instance of an abstract syntax rule.
 */

#include <stdio.h>
#include "util.h"
#include "symbol.h" /* symbol table data structures */
#include "absyn.h"  /* abstract syntax data structures */
#include "prabsyn.h" /* function prototype */

/* local function prototypes */
static void pr_var(FILE *out, AST_var v, int d);
static void pr_dec(FILE *out, AST_dec v, int d);
static void pr_ty(FILE *out, AST_ty v, int d);
static void pr_expList(FILE *out, AST_expList v, int d);
static void pr_decList(FILE *out, AST_decList v, int d);
static void pr_namety(FILE *out, AST_namety v, int d);
static void pr_nametyList(FILE *out, AST_nametyList v, int d);


static void indent(FILE *out, int d) {
 int i;
 for (i = 0; i <= d; i++) fprintf(out, " ");
}

/* Print AST_var types. Indent d spaces. */
static void pr_var(FILE *out, AST_var v, int d) {
 indent(out, d);
 switch (v->kind) {
 case AST_simpleVar:
   fprintf(out, "simpleVar(%s)", SB_name(v->u.simple)); 
   break;
 case AST_fieldVar:
   fprintf(out, "%s\n", "fieldVar(");
   pr_var(out, v->u.field.var, d+1); fprintf(out, "%s\n", ","); 
   indent(out, d+1); fprintf(out, "%s)", SB_name(v->u.field.sym));
   break;
 case AST_subscriptVar:
   fprintf(out, "%s\n", "subscriptVar(");
   pr_var(out, v->u.subscript.var, d+1); fprintf(out, "%s\n", ","); 
   pr_exp(out, v->u.subscript.exp, d+1); fprintf(out, "%s", ")");
   break;
 default:
   assert(0); 
 } 
}

static char str_oper[][12] = {
   "PLUS", "MINUS", "TIMES", "DIVIDE", 
   "EQUAL", "NOTEQUAL", "LESSTHAN", "LESSEQ", "GREAT", "GREATEQ"};
 
static void pr_oper(FILE *out, AST_oper d) {
  fprintf(out, "%s", str_oper[d]);
}

/* Print AST_var types. Indent d spaces. */
void pr_exp(FILE *out, AST_exp v, int d) {
 indent(out, d);
 switch (v->kind) {
 case AST_varExp:
   fprintf(out, "varExp(\n"); pr_var(out, v->u.var, d+1); 
   fprintf(out, "%s", ")");
   break;
 case AST_nilExp:
   fprintf(out, "nilExp()");
   break;
 case AST_intExp:
   fprintf(out, "intExp(%d)", v->u.intt);
   break;
 case AST_stringExp:
   fprintf(out, "stringExp(%s)", v->u.stringg);
   break;
 case AST_opExp:
   fprintf(out, "opExp(\n");
   indent(out, d+1); pr_oper(out, v->u.op.oper); fprintf(out, ",\n"); 
   pr_exp(out, v->u.op.left, d+1); fprintf(out, ",\n"); 
   pr_exp(out, v->u.op.right, d+1); fprintf(out, ")");
   break;
 case AST_seqExp:
   fprintf(out, "seqExp(\n");
   pr_expList(out, v->u.seq, d+1); fprintf(out, ")");
   break;
 case AST_assignExp:
   fprintf(out, "assignExp(\n");
   pr_var(out, v->u.assign.var, d+1); fprintf(out, ",\n");
   pr_exp(out, v->u.assign.exp, d+1); fprintf(out, ")");
   break;
 case AST_ifExp:
   fprintf(out, "iffExp(\n");
   pr_exp(out, v->u.iff.test, d+1); fprintf(out, ",\n");
   pr_exp(out, v->u.iff.then, d+1);
   if (v->u.iff.elsee) { /* else is optional */
      fprintf(out, ",\n");
      pr_exp(out, v->u.iff.elsee, d+1);
   }
   fprintf(out, ")");
   break;
 case AST_whileExp:
   fprintf(out, "whileExp(\n");
   pr_exp(out, v->u.whilee.test, d+1); fprintf(out, ",\n");
   pr_exp(out, v->u.whilee.body, d+1); fprintf(out, ")\n");
   break;
 case AST_forExp:
   fprintf(out, "forExp(%s,\n", SB_name(v->u.forr.var)); 
   pr_exp(out, v->u.forr.lo, d+1); fprintf(out, ",\n");
   pr_exp(out, v->u.forr.hi, d+1); fprintf(out, "%s\n", ",");
   pr_exp(out, v->u.forr.body, d+1); fprintf(out, ",\n");
   indent(out, d+1); fprintf(out, "%s", v->u.forr.escape ? "TRUE)" : "FALSE)");
   break;
 case AST_letExp:
   fprintf(out, "letExp(\n");
   pr_decList(out, v->u.let.decs, d+1); fprintf(out, ",\n");
   pr_exp(out, v->u.let.body, d+1); fprintf(out, ")");
   break;
 case AST_arrayExp:
   fprintf(out, "arrayExp(%s,\n", SB_name(v->u.array.typ));
   pr_exp(out, v->u.array.size, d+1); fprintf(out, ",\n");
   pr_exp(out, v->u.array.init, d+1); fprintf(out, ")");
   break;
 default:
   assert(0); 
 } 
}

static void pr_dec(FILE *out, AST_dec v, int d) {
 indent(out, d);
 switch (v->kind) {
 case AST_varDec:
   fprintf(out, "varDec(%s,\n", SB_name(v->u.var.var));
   if (v->u.var.typ) {
     indent(out, d+1); fprintf(out, "%s,\n", SB_name(v->u.var.typ)); 
   }
   pr_exp(out, v->u.var.init, d+1); fprintf(out, ",\n");
   indent(out, d+1); fprintf(out, "%s", v->u.var.escape ? "TRUE)" : "FALSE)");
   break;
 case AST_typeDec:
   fprintf(out, "typeDec(\n"); 
   pr_nametyList(out, v->u.type, d+1); fprintf(out, ")");
   break;
 default:
   assert(0); 
 } 
}

static void pr_ty(FILE *out, AST_ty v, int d) {
 indent(out, d);
 switch (v->kind) {
 case AST_nameTy:
   fprintf(out, "nameTy(%s)", SB_name(v->u.name));
   break;
 case AST_arrayTy:
   fprintf(out, "arrayTy(%s)", SB_name(v->u.array));
   break;
 default:
   assert(0); 
 } 
}


static void pr_expList(FILE *out, AST_expList v, int d) {
 indent(out, d);
 if (v) {
   fprintf(out, "expList(\n"); 
   pr_exp(out, v->head, d+1); fprintf(out, ",\n");
   pr_expList(out, v->tail, d+1);
   fprintf(out, ")");
 }
 else fprintf(out, "expList()"); 

}

static void pr_decList(FILE *out, AST_decList v, int d) {
 indent(out, d);
 if (v) {
   fprintf(out, "decList(\n"); 
   pr_dec(out, v->head, d+1); fprintf(out, ",\n");
   pr_decList(out, v->tail, d+1);
   fprintf(out, ")");
 }
 else fprintf(out, "decList()"); 

}

static void pr_namety(FILE *out, AST_namety v, int d) {
 indent(out, d);
 fprintf(out, "namety(%s,\n", SB_name(v->name)); 
 pr_ty(out, v->ty, d+1); fprintf(out, ")");
}

static void pr_nametyList(FILE *out, AST_nametyList v, int d) {
 indent(out, d);
 if (v) {
   fprintf(out, "nametyList(\n"); 
   pr_namety(out, v->head, d+1); fprintf(out, ",\n");
   pr_nametyList(out, v->tail, d+1); fprintf(out, ")");
 }
 else fprintf(out, "nametyList()");
}






