/*********************************************************************************
*FileName: temp.c
*Author: Debbie Xie
*Create Time: 2017/5/30
*Description: Functions to create and manipulate temporary variables which are
              used in the IR tree representation before it has been determined
              which variables are to go into registers.
              Labels are used to mark where to jump.
              Temps are temporary variables.
**********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"

/*Given TMP_label,
 *return it's string */
string TMP_labelstring(TMP_label strLabel){
    return SB_name(strLabel);
}

static int labelNum = 0;

/*New a label,
 *increase labelNum by 1
 *add it into hashtable*/
TMP_label TMP_newlabel(){
    char buffer[100];
    sprintf(buffer, "L%d", labelNum++);
    return TMP_namedlabel(String(buffer));
}

TMP_label TMP_namedlabel(string name){
    return SB_Symbol(name);
}

static int temps = 100;
struct TMP_temp_ {int num;};

TMP_temp TMP_newtemp(){
    TMP_temp ptr = (TMP_temp)check_malloc(sizeof(*ptr));
    ptr->num = temps++;
    char num[16];
    sprintf(num, "%d", ptr->num);
    TMP_enter(TMP_name(), ptr, String(num));
    return ptr;
}

struct TMP_map_ {TB_table tab; TMP_map under;};

/*????????????????????????????*/
TMP_map TMP_name(){
    static TMP_map map = NULL;//??????????
    if(!map) map = TMP_empty();
    return map;
}

TMP_map newMap(TB_table table, TMP_map under){
    TMP_map map = check_malloc(sizeof(struct TMP_map_));
    map->tab = table;
    map->under = under;
    return map;
}

TMP_map TMP_empty(){
    return newMap(TB_create(), NULL);
}

/*????????????????????????????????????????*/
TMP_map TMP_layerMap(TMP_map overM, TMP_map underM){
    if(overM == NULL){
        return underM;
    }
    else{
        return newMap(overM->tab, TMP_layerMap(overM->under, underM));
    }
}

void TMP_enter(TMP_map map, TMP_temp te, string str){
    assert(map && map->tab);
    TB_push(map->tab, te, str);
}

string TMP_look(TMP_map map, TMP_temp te){
    string str;
    assert(map && map->tab);
    str = TB_look(map->tab, te);
    if(str)
        return str;
    else if(map->under)
        return TMP_look(map->under, te);
    else
        return NULL;
}

TMP_tempList TMP_TempList(TMP_temp head, TMP_tempList tail) 
{TMP_tempList ptr = (TMP_tempList) check_malloc(sizeof (*ptr));
 ptr->head=head; ptr->tail=tail;
 return ptr;
}

TMP_labelList TMP_LabelList(TMP_label head, TMP_labelList tail)
{TMP_labelList ptr = (TMP_labelList) check_malloc(sizeof (*ptr));
 ptr->head=head; ptr->tail=tail;
 return ptr;
}





