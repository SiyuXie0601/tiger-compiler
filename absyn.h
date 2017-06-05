/*
 * absyn.h - Abstract Syntax Header (Chapter 4)
 *
 * All types and functions declared in this header file begin with "AST_"
 * Linked list types end with "..list"
 */

/* Type Definitions */
#ifndef _ABSYN_H_
#define _ABSYN_H_
#include"symbol.h"
typedef int AST_pos;

typedef struct AST_var_ *AST_var;
typedef struct AST_exp_ *AST_exp;
typedef struct AST_dec_ *AST_dec;
typedef struct AST_ty_ *AST_ty;

typedef struct AST_decList_ *AST_decList;
typedef struct AST_expList_ *AST_expList;
typedef struct AST_field_ *AST_field;
typedef struct AST_fieldList_ *AST_fieldList;
typedef struct AST_fundec_ *AST_fundec;
typedef struct AST_fundecList_ *AST_fundecList;
typedef struct AST_namety_ *AST_namety;
typedef struct AST_nametyList_ *AST_nametyList;
typedef struct AST_efield_ *AST_efield;
typedef struct AST_efieldList_ *AST_efieldList;



typedef enum {AST_plusOp, AST_minusOp, AST_timesOp, AST_divideOp,
	     AST_eqOp, AST_neqOp, AST_ltOp, AST_leOp, AST_gtOp, AST_geOp} AST_oper;

struct AST_var_
       {enum {AST_simpleVar, AST_fieldVar, AST_subscriptVar} kind;
        AST_pos pos;
	union {SB_symbol simple;
	       struct {AST_var var;
		       SB_symbol sym;} field;
	       struct {AST_var var;
		       AST_exp exp;} subscript;
	     } u;
      };

struct AST_exp_
      {enum {AST_varExp, AST_nilExp, AST_intExp, AST_stringExp, AST_callExp,
	       AST_opExp, AST_recordExp, AST_seqExp, AST_assignExp, AST_ifExp,
	       AST_whileExp, AST_forExp, AST_breakExp, AST_letExp, AST_arrayExp} kind;
       AST_pos pos;
       union {AST_var var;
	      /* nil; - needs only the pos */
	      int intt;
	      string stringg;
	      struct {SB_symbol func; AST_expList args;} call;
	      struct {AST_oper oper; AST_exp left; AST_exp right;} op;
	      struct {SB_symbol typ; AST_efieldList fields;} record;
	      AST_expList seq;
	      struct {AST_var var; AST_exp exp;} assign;
	      struct {AST_exp test, then, elsee;} iff; /* elsee is optional */
	      struct {AST_exp test, body;} whilee;
	      struct {SB_symbol var; AST_exp lo,hi,body; bool escape;} forr;
	      /* breakk; - need only the pos */
	      struct {AST_decList decs; AST_exp body;} let;
	      struct {SB_symbol typ; AST_exp size, init;} array;
	    } u;
     };

struct AST_dec_ 
    {enum {AST_functionDec, AST_varDec, AST_typeDec} kind;
     AST_pos pos;
     union {AST_fundecList function;
 	    /* escape may change after the initial declaration */
	    struct {SB_symbol var; SB_symbol typ; AST_exp init; bool escape;} var;
	    AST_nametyList type;
	  } u;
   };

struct AST_ty_ {enum {AST_nameTy, AST_recordTy, AST_arrayTy} kind;
	      AST_pos pos;
	      union {SB_symbol name;
		     AST_fieldList record;
		     SB_symbol array;
		   } u;
	    };

/* Linked lists and nodes of lists */

struct AST_field_ {SB_symbol name, typ; AST_pos pos; bool escape;};
struct AST_fieldList_ {AST_field head; AST_fieldList tail;};
struct AST_expList_ {AST_exp head; AST_expList tail;};
struct AST_fundec_ {AST_pos pos;
                 SB_symbol name; AST_fieldList params; 
		 SB_symbol result; AST_exp body;};

struct AST_fundecList_ {AST_fundec head; AST_fundecList tail;};
struct AST_decList_ {AST_dec head; AST_decList tail;};
struct AST_namety_ {SB_symbol name; AST_ty ty;};
struct AST_nametyList_ {AST_namety head; AST_nametyList tail;};
struct AST_efield_ {SB_symbol name; AST_exp exp;};
struct AST_efieldList_ {AST_efield head; AST_efieldList tail;};


/* Function Prototypes */
AST_var AST_SimpleVar(AST_pos pos, SB_symbol sym);
AST_var AST_FieldVar(AST_pos pos, AST_var var, SB_symbol sym);
AST_var AST_SubscriptVar(AST_pos pos, AST_var var, AST_exp exp);
AST_exp AST_VarExp(AST_pos pos, AST_var var);
AST_exp AST_NilExp(AST_pos pos);
AST_exp AST_IntExp(AST_pos pos, int i);
AST_exp AST_StringExp(AST_pos pos, string s);
AST_exp AST_CallExp(AST_pos pos, SB_symbol func, AST_expList args);
AST_exp AST_OpExp(AST_pos pos, AST_oper oper, AST_exp left, AST_exp right);
AST_exp AST_RecordExp(AST_pos pos, SB_symbol typ, AST_efieldList fields);
AST_exp AST_SeqExp(AST_pos pos, AST_expList seq);
AST_exp AST_AssignExp(AST_pos pos, AST_var var, AST_exp exp);
AST_exp AST_IfExp(AST_pos pos, AST_exp test, AST_exp then, AST_exp elsee);
AST_exp AST_WhileExp(AST_pos pos, AST_exp test, AST_exp body);
AST_exp AST_ForExp(AST_pos pos, SB_symbol var, AST_exp lo, AST_exp hi, AST_exp body);
AST_exp AST_BreakExp(AST_pos pos);
AST_exp AST_LetExp(AST_pos pos, AST_decList decs, AST_exp body);
AST_exp AST_ArrayExp(AST_pos pos, SB_symbol typ, AST_exp size, AST_exp init);
AST_dec AST_FunctionDec(AST_pos pos, AST_fundecList function);
AST_dec AST_VarDec(AST_pos pos, SB_symbol var, SB_symbol typ, AST_exp init);
AST_dec AST_TypeDec(AST_pos pos, AST_nametyList type);
AST_ty AST_NameTy(AST_pos pos, SB_symbol name);
AST_ty AST_RecordTy(AST_pos pos, AST_fieldList record);
AST_ty AST_ArrayTy(AST_pos pos, SB_symbol array);
AST_field AST_Field(AST_pos pos, SB_symbol name, SB_symbol typ);
AST_fieldList AST_FieldList(AST_field head, AST_fieldList tail);
AST_expList AST_ExpList(AST_exp head, AST_expList tail);
AST_fundec AST_Fundec(AST_pos pos, SB_symbol name, AST_fieldList params, SB_symbol result,
		  AST_exp body);
AST_fundecList AST_FundecList(AST_fundec head, AST_fundecList tail);
AST_decList AST_DecList(AST_dec head, AST_decList tail);
AST_namety AST_Namety(SB_symbol name, AST_ty ty);
AST_nametyList AST_NametyList(AST_namety head, AST_nametyList tail);
AST_efield AST_Efield(SB_symbol name, AST_exp exp);
AST_efieldList AST_EfieldList(AST_efield head, AST_efieldList tail);


/*************************************
 print routines
*************************************/ 
void print_SB_symbol(SB_symbol s);

void print_absyn_exp(AST_exp exp, int indent);
void print_absyn_var(AST_var var, int indent);
void print_absyn_dec(AST_dec dec, int indent);
void print_absyn_ty(AST_ty, int indent);


void print_SimpleVar(AST_var var, int indent);
void print_FieldVar(AST_var var, int indent);
void print_SubscriptVar(AST_var var, int indent);

void print_VarExp(AST_exp exp, int indent);
void print_NilExp(AST_exp exp, int indent);
void print_IntExp(AST_exp exp, int indent);
void print_StringExp(AST_exp exp, int indent);
void print_CallExp(AST_exp exp, int indent);
void print_OpExp(AST_exp exp, int indent);
void print_RecordExp(AST_exp exp, int indent);
void print_SeqExp(AST_exp exp, int indent);
void print_AssignExp(AST_exp exp, int indent);
void print_IfExp(AST_exp exp, int indent);
void print_WhileExp(AST_exp exp, int indent);
void print_ForExp(AST_exp exp, int indent);
void print_BreakExp(AST_exp exp, int indent);
void print_LetExp(AST_exp exp, int indent);
void print_ArrayExp(AST_exp exp, int indent);

void print_FunctionDec(AST_dec dec, int indent);
void print_VarDec(AST_dec dec, int indent);
void print_TypeDec(AST_dec dec, int indent);

void print_NameTy(AST_ty ty, int indent);
void print_RecordTy(AST_ty ty, int indent);
void print_ArrayTy(AST_ty ty, int indent);

void print_Field(AST_field field, int indent);
void print_Fundec(AST_fundec fundec, int indent);
void print_Namety(AST_namety namety, int indent);
void print_Efield(AST_efield efield, int indent);

void print_EfieldList(AST_efieldList efieldList, int indent);
void print_FieldList(AST_fieldList fieldList, int indent);
void print_ExpList(AST_expList expList, int indent);
void print_NametyList(AST_nametyList nametyList, int indent);
void print_FundecList(AST_fundecList fundecList, int indent);
void print_DecList(AST_decList decList, int indent);

void print_indent(int indent);
void print_end(); 

#endif
