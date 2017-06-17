/*********************************************************************************
*FileName: assem.c
*Author: Debbie Xie
*Create Time: 2017/6/16
*Description: Called by mipscodegen.c
**********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "errormsg.h"

ASSB_targets ASSB_Targets(TMP_labelList lbls){
    ASSB_targets ptr = check_malloc(sizeof *ptr);
    ptr->labels = lbls;
    return ptr;
}

ASSB_instr ASSB_Oper(string str, TMP_tempList dst, TMP_tempList src, ASSB_targets jumps){
    ASSB_instr ptr = (ASSB_instr)check_malloc(sizeof *ptr);
    ptr->kind = I_OPER;
    ptr->u.OPER.assem = str;
    ptr->u.OPER.dst = dst;
    ptr->u.OPER.src = src;
    ptr->u.OPER.jumps = jumps;
    return ptr;
}

ASSB_instr ASSB_Label(string str, TMP_label lbl){
    ASSB_instr ptr = (ASSB_instr)check_malloc(sizeof *ptr);
    ptr->kind = I_LABEL;
    ptr->u.LABEL.assem = str;
    ptr->u.LABEL.label = lbl;
    return ptr;
}

ASSB_instr ASSB_Move(string str, TMP_tempList dst, TMP_tempList src){
    ASSB_instr ptr = (ASSB_instr)check_malloc(sizeof *ptr);
    ptr->kind = I_MOVE;
    ptr->u.MOVE.assem = str;
    ptr->u.MOVE.dst = dst;
    ptr->u.MOVE.src = src;
    return ptr;
}

ASSB_instrList ASSB_InstrList(ASSB_instr h, ASSB_instrList t){
    ASSB_instrList ptr = (ASSB_instrList)check_malloc(sizeof *ptr);
    ptr->head = h;
    ptr->tail = t;
    return ptr;
}

ASSB_instrList ASSB_splice(ASSB_instrList first, ASSB_instrList second){
    ASSB_instrList ptr;
    if(first == NULL)
        return second;
    for(ptr = first; ptr->tail != NULL; ptr = ptr->tail);
    ptr->tail = second;
    return first;
}

static TMP_temp nthTemp(TMP_tempList l, int th){
    if(th == 0)
        return l->head;
    else
        return nthTemp(l->tail, th - 1);
}

static TMP_label nthLabel(TMP_labelList l, int th){
    if(th == 0)
        return l->head;
    else
        return nthLabel(l->tail, th - 1);
}

/*put string created by this fun by reading assb and replacing `d `t `jumps
  into rst
  map is used to determine what to do with each temp
*/
void ASSB_format(char *rst, string assb, TMP_tempList dest, TMP_tempList source, ASSB_targets jumps, TMP_map map){
    char *ptr;
    int j = 0;
    for(ptr = assb; ptr && *ptr != '\0'; ptr++){
        if(*ptr == '`'){
            switch(*(++ptr)){
                case 's':
                    {
                        int k = atoi(++ptr);
                        string str = TMP_look(map, nthTemp(source, k));
                        strcpy(rst + j, str);
                        j += strlen(str);
                    }
                    break;
                case 'd':
                    {
                        int k = atoi(++ptr);
                        string str = TMP_look(map, nthTemp(dest, k));
                        strcpy(rst + j, str);
                        j += strlen(str);
                    }
                    break;
                case 'j':
                    {
                        int k = atoi(++ptr);
                        string str = TMP_labelstring(nthLabel(jumps->labels, k));
                        strcpy(rst + j, str);
                        j += strlen(str);
                    }
                    break;
                case '`': 
                    {
                        rst[j] = '`'; 
                        j++; 
                    }
                    break;
                default:
                    assert(0);
            }
        }
        else{
            rst[j] == *ptr;
            j++;
        }
    }
    rst[j] = '\0';
}

void ASSB_print(FILE *out, ASSB_instr instr, TMP_map map){
    char result[200];
    switch(instr->kind){
        case I_OPER:
            {
                ASSB_format(result, instr->u.OPER.assem, instr->u.OPER.dst, instr->u.OPER.src, instr->u.OPER.jumps, map);        
                fprintf(out, "%s", result);
            }
            break;
        case I_LABEL:
            {
                ASSB_format(result, instr->u.LABEL.assem, NULL, NULL, NULL, map);        
                fprintf(out, "%s", result);
            }
            break;
        case I_MOVE:
            {
                ASSB_format(result, instr->u.OPER.assem, instr->u.OPER.dst, instr->u.OPER.src, NULL, map);        
                fprintf(out, "%s", result);
            }
            break;          
    }
}

void ASSB_printInstrList(FILE *out, ASSB_instrList instrList, TMP_map map){
    for(; instrList; instrList = instrList->tail){
        ASSB_print(out, instrList->head, map);
    }
}

ASSB_proc ASSB_Proc(string str, ASSB_instrList body, string exp){
    ASSB_proc proc = checked_malloc(sizeof(*proc));
    proc->prolog=str; proc->body=body; proc->epilog=exp;
    return proc;
}