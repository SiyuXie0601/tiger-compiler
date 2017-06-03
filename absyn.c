/*
 * absyn.c - Abstract Syntax Functions. Most functions create an instance of an
 *           abstract syntax rule.
 */

#include "util.h"
#include "absyn.h"  /* abstract syntax data structures */
#include "stdio.h"
#include "constants.h"




AST_var AST_SimpleVar(AST_pos pos, SB_symbol sym)
{AST_var p = check_malloc(sizeof(*p));
 p->kind=AST_simpleVar;
 p->pos=pos;
 p->u.simple=sym;
 return p;
}

AST_var AST_FieldVar(AST_pos pos, AST_var var, SB_symbol sym)
{AST_var p = check_malloc(sizeof(*p));
 p->kind=AST_fieldVar;
 p->pos=pos;
 p->u.field.var=var;
 p->u.field.sym=sym;
 return p;
}

AST_var AST_SubscriptVar(AST_pos pos, AST_var var, AST_exp exp)
{AST_var p = check_malloc(sizeof(*p));
 p->kind=AST_subscriptVar;
 p->pos=pos;
 p->u.subscript.var=var;
 p->u.subscript.exp=exp;
 return p;
}


AST_exp AST_VarExp(AST_pos pos, AST_var var)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_varExp;
 p->pos=pos;
 p->u.var=var;
 return p;
}

AST_exp AST_NilExp(AST_pos pos)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_nilExp;
 p->pos=pos;
 return p;
}

AST_exp AST_IntExp(AST_pos pos, int i)
{ AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_intExp;
 p->pos=pos;
 p->u.intt=i;
 return p;
}

AST_exp AST_StringExp(AST_pos pos, string s)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_stringExp;
 p->pos=pos;
 p->u.stringg=s;
 return p;
}

AST_exp AST_OpExp(AST_pos pos, AST_oper oper, AST_exp left, AST_exp right)
{
	
	AST_exp p = check_malloc(sizeof(*p));
	p->kind = AST_opExp;
	p->pos = pos;
	p->u.op.oper = oper;
	p->u.op.left = left;
	p->u.op.right = right;
	return p;

}



AST_exp AST_SeqExp(AST_pos pos, AST_expList seq)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_seqExp;
 p->pos=pos;
 p->u.seq=seq;
 return p;
}

AST_exp AST_AssignExp(AST_pos pos, AST_var var, AST_exp exp)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_assignExp;
 p->pos=pos;
 p->u.assign.var=var;
 p->u.assign.exp=exp;
 //Vin-edit removed const allocation from here
 return p;
}

AST_exp AST_IfExp(AST_pos pos, AST_exp test, AST_exp then, AST_exp elsee)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_ifExp;
 p->pos=pos;
 p->u.iff.test=test;
 p->u.iff.then=then;
 p->u.iff.elsee=elsee;
 return p;
}

AST_exp AST_WhileExp(AST_pos pos, AST_exp test, AST_exp body)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_whileExp;
 p->pos=pos;
 p->u.whilee.test=test;
 p->u.whilee.body=body;
 return p;
}

AST_exp AST_ForExp(AST_pos pos, SB_symbol var, AST_exp lo, AST_exp hi, AST_exp body)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_forExp;
 p->pos=pos;
 p->u.forr.var=var;
 p->u.forr.lo=lo;
 p->u.forr.hi=hi;
 p->u.forr.body=body;
 p->u.forr.escape=TRUE;
 return p;
}



AST_exp AST_LetExp(AST_pos pos, AST_decList decs, AST_exp body)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_letExp;
 p->pos=pos;
 p->u.let.decs=decs;
 p->u.let.body=body;
 return p;
}

AST_exp AST_ArrayExp(AST_pos pos, SB_symbol typ, AST_exp size, AST_exp init)
{AST_exp p = check_malloc(sizeof(*p));
 p->kind=AST_arrayExp;
 p->pos=pos;
 p->u.array.typ=typ;
 p->u.array.size=size;
 p->u.array.init=init;
 return p;
}


AST_dec AST_VarDec(AST_pos pos, SB_symbol var, SB_symbol typ, AST_exp init)
{AST_dec p = check_malloc(sizeof(*p));
 p->kind=AST_varDec;
 p->pos=pos;
 p->u.var.var=var;
 p->u.var.typ=typ;
 p->u.var.init=init;
 p->u.var.escape=TRUE;
// removed Constant table entry form here
	 
 return p;
}

AST_dec AST_TypeDec(AST_pos pos, AST_nametyList type)
{AST_dec p = check_malloc(sizeof(*p));
 p->kind=AST_typeDec;
 p->pos=pos;
 p->u.type=type;
 return p;
}


AST_ty AST_ArrayTy(AST_pos pos, SB_symbol array)
{AST_ty p = check_malloc(sizeof(*p));
 p->kind=AST_arrayTy;
 p->pos=pos;
 p->u.array=array;
 return p;
}


AST_expList AST_ExpList(AST_exp head, AST_expList tail)
{AST_expList p = check_malloc(sizeof(*p));
 p->head=head;
 p->tail=tail;
 return p;
}



AST_decList AST_DecList(AST_dec head, AST_decList tail)
{AST_decList p = check_malloc(sizeof(*p));
 p->head=head;
 p->tail=tail;
 return p;
}

AST_namety AST_Namety(SB_symbol name, AST_ty ty)
{AST_namety p = check_malloc(sizeof(*p));
 p->name=name;
 p->ty=ty;
 return p;
}

AST_nametyList AST_NametyList(AST_namety head, AST_nametyList tail)
{AST_nametyList p = check_malloc(sizeof(*p));
 p->head=head;
 p->tail=tail;
 return p;
}



void print_SB_symbol(SB_symbol s) {
  if(s == NULL) 
     printf("UnDefined");
  else 
     printf("%s", SB_name(s));
}


void print_absyn_exp(AST_exp exp, int indent) {
  if(exp == NULL) return; 
  switch(exp->kind) {
  case AST_varExp:
    print_VarExp(exp, indent); 
    break;
  case AST_nilExp:
    print_NilExp(exp, indent); 
    break;
  case AST_intExp:
    print_IntExp(exp, indent); 
    break;
  case AST_stringExp:
    print_StringExp(exp, indent); 
    break;
  case AST_callExp:
    print_CallExp(exp, indent); 
    break;
  case AST_opExp:
    print_OpExp(exp, indent); 
    break;
  case AST_seqExp:
    print_SeqExp(exp, indent); 
    break;
  case AST_assignExp:
    print_AssignExp(exp, indent); 
    break;
  case AST_ifExp:
    print_IfExp(exp, indent); 
    break;
  case AST_whileExp:
     print_WhileExp(exp, indent); 
    break;
  case AST_forExp:
    print_ForExp(exp, indent); 
    break;
  case AST_letExp:
    print_LetExp(exp, indent); 
    break;
  case AST_arrayExp:
    print_ArrayExp(exp, indent); 
    break;
  default:
    printf("Illegal Expression");
  }
}

void print_absyn_var(AST_var var, int indent) {
  if(var == 0) return; 
  switch(var->kind) {
  case AST_simpleVar:
    print_SimpleVar(var, indent);
    break;
  case AST_subscriptVar:
    print_SubscriptVar(var, indent);
    break;
  default:
    printf("Illegal Var"); 
  }
}

void print_absyn_dec(AST_dec dec, int indent) {
  if(dec == 0) return; 
  switch (dec->kind) { 
  case AST_varDec:
    print_VarDec(dec, indent);
    break; 
  case AST_typeDec:
    print_TypeDec(dec, indent);
    break; 
  default:
    printf("Illegal Dec");
  }
}

void print_absyn_ty(AST_ty ty, int indent) {
  if(ty == 0) return; 
  switch(ty->kind) {
  case AST_nameTy:
    print_NameTy(ty, indent);
    break; 
  case AST_arrayTy:
    print_ArrayTy(ty, indent);
    break;
  default:
    printf("Illegal Ty");
  }
}

void print_absyn_oper(AST_oper oper) {
  switch(oper) {
  case AST_plusOp:
    printf("+");
    break;
  case AST_minusOp:
    printf("-");
    break;
  case AST_timesOp:
    printf("*");
    break;
  case AST_divideOp:
    printf("/");
    break;
  case AST_eqOp:
    printf("=");
    break; 
  case AST_neqOp:
    printf("<>");
    break;
  case AST_ltOp:
    printf("<");
    break; 
  case AST_leOp:
    printf("<=");
    break;
  case AST_gtOp:
    printf(">");
    break;
  case AST_geOp:
    printf(">=");
    break;
  default:
    printf("Illegal Operator"); 
  }
}


void print_SimpleVar(AST_var var, int indent){
  print_indent(indent);
  
  printf("SimpleVar: "); 
  print_SB_symbol(var->u.simple);
  print_end(); 
}

void print_SubscriptVar(AST_var var, int indent) {
  print_indent(indent);
  printf("SubscriptVar: ");

  print_indent(indent);
  printf(" var = ");
  print_absyn_var(var->u.subscript.var, indent + 1);

  print_indent(indent);
  printf(" exp = ");
  print_absyn_exp(var->u.subscript.exp, indent + 1);

  print_end();  
}

void print_VarExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("VarExp: ");
  print_absyn_var(exp->u.var, indent + 1);

  print_end(); 
}

void print_NilExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("NilExp");

  print_end(); 
}

void print_IntExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("IntExp: %d", exp->u.intt);
  print_end(); 
}

void print_StringExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("StringExp: %s", exp->u.stringg);

  print_end(); 
}

void print_CallExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("CallExp: ");

  print_indent(indent);
  printf(" func = ");
  print_SB_symbol(exp->u.call.func);

  print_indent(indent);
  printf(" args = ");
  print_ExpList(exp->u.call.args, indent + 1);

  print_end(); 
}

void print_OpExp(AST_exp exp, int indent) {
  print_indent(indent);

  printf("OpExp ("); 
  print_absyn_oper(exp->u.op.oper);
  printf(")"); 

  print_indent(indent);
  printf(" left"); 
  print_absyn_exp(exp->u.op.left, indent + 1);

  print_indent(indent);
  printf(" right"); 
  print_absyn_exp(exp->u.op.right, indent + 1);

  print_end(); 
}


void print_SeqExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("SeqExp: ");
  print_ExpList(exp->u.seq, indent + 1); 

  print_end(); 
}

void print_AssignExp(AST_exp exp, int indent) {
  print_indent(indent);

  printf("Assign: ");

  print_indent(indent);
  printf(" var = ");
  print_absyn_var(exp->u.assign.var, indent + 1);

  print_indent(indent);
  printf(" exp = ");
  print_absyn_exp(exp->u.assign.exp, indent + 1);  

  print_end(); 
}

void print_IfExp(AST_exp exp, int indent) {
  print_indent(indent);

  printf("If: ");

  print_indent(indent);
  printf(" test = ");
  print_absyn_exp(exp->u.iff.test, indent + 1);

  print_indent(indent);
  printf(" then = ");
  print_absyn_exp(exp->u.iff.then, indent + 1);

  print_indent(indent);
  printf(" else = ");
  print_absyn_exp(exp->u.iff.elsee, indent + 1); 

  print_end();
}

void print_WhileExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("While:");

  print_indent(indent);
  printf(" test = "); 
  print_absyn_exp(exp->u.whilee.test, indent + 1);

  print_indent(indent);
  printf(" body = "); 
  print_absyn_exp(exp->u.whilee.body, indent + 1);

  print_end();
}

void print_ForExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("For: %d", exp->u.forr.escape);

  print_indent(indent);
  printf(" escape = %d", exp->u.forr.escape);

  print_indent(indent);
  printf(" var = ");
  print_SB_symbol(exp->u.forr.var);
  
  printf(" low = "); 
  print_indent(indent);
  print_absyn_exp(exp->u.forr.lo, indent + 1);
  
  printf(" high = "); 
  print_indent(indent);
  print_absyn_exp(exp->u.forr.hi, indent + 1);
  
  printf(" body "); 
  print_indent(indent);
  print_absyn_exp(exp->u.forr.body, indent + 1);
  
  print_end();
}

void print_LetExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("Let: ");

  print_indent(indent);
  printf(" decs = ");
  print_DecList(exp->u.let.decs, indent + 1);

  print_indent(indent);
  printf(" body = ");
  print_absyn_exp(exp->u.let.body, indent + 1); 

  print_end();
}

void print_ArrayExp(AST_exp exp, int indent) {
  print_indent(indent);
  printf("Array: ");

  print_indent(indent);
  printf(" type = ");
  print_SB_symbol(exp->u.array.typ);
  
  print_indent(indent);
  printf(" size = "); 
  print_absyn_exp(exp->u.array.size, indent + 1);
  
  print_indent(indent);
  printf(" init = ");
  print_absyn_exp(exp->u.array.init, indent + 1); 

  print_end();
}

void print_VarDec(AST_dec dec, int indent) {
  print_indent(indent);
  printf("VarDec: ");
  
  print_indent(indent);
  printf(" name = ");
  print_SB_symbol(dec->u.var.var);

  print_indent(indent);
  printf(" escape = %d ", dec->u.var.escape);

  print_indent(indent);
  printf(" type = ");
  print_SB_symbol(dec->u.var.typ);

  print_indent(indent);
  printf(" init = ");
  print_absyn_exp(dec->u.var.init, indent + 1);
  
  print_end();
}

void print_TypeDec(AST_dec dec, int indent) {
  print_indent(indent);
  printf("TypeDec: ");

  print_NametyList(dec->u.type, indent + 1);
  print_end();
}

void print_NameTy(AST_ty ty, int indent) {
  print_indent(indent);

  printf("NameTy: ");
  print_SB_symbol(ty->u.name);

  print_end();
}


void print_ArrayTy(AST_ty ty, int indent) {
  print_indent(indent);

  printf("ArrayTy: ");
  print_SB_symbol(ty->u.array);
  
  print_end();
}




void print_Namety(AST_namety namety, int indent) {
  print_indent(indent);
  printf("Namety: ");

  print_indent(indent);
  printf(" name = ");
  print_SB_symbol(namety->name);

  print_indent(indent);
  printf(" type = ");
  print_absyn_ty(namety->ty, indent + 1);
  
  print_end();
}



void print_ExpList(AST_expList expList, int indent) {
  if(expList == NULL) return; 
  print_absyn_exp(expList->head, indent);
  print_ExpList(expList->tail, indent);   
}

void print_NametyList(AST_nametyList nametyList, int indent) {
  if(nametyList == NULL) return; 
  print_Namety(nametyList->head, indent);
  print_NametyList(nametyList->tail, indent); 
}


void print_DecList(AST_decList decList, int indent) {
  if(decList == NULL) return; 
  print_absyn_dec(decList->head, indent);
  print_DecList(decList->tail, indent); 
}


void print_indent(int indent) {
  int i;
  printf("\n");
  for (i = 0; i < indent; i++)
    printf("    ");
}

void print_end() {
}
