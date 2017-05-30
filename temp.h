/*********************************************************************************
*FileName: temp.h
*Author: Debbie Xie
*Create Time: 2017/5/30
*Description: 
**********************************************************************************/

#ifndef _TEMP_H_
#define _TEMP_H_

#include "symbol.h"
#include "table.h"

/*Label*/
typedef SB_symbol TMP_label;
string TMP_labelstring(TMP_label strLabel);
TMP_label TMP_newlabel();
TMP_label TMP_namedlabel(string name);

typedef struct TMP_labelList_ *TMP_labelList;
struct TMP_labelList_ { TMP_label head; TMP_labelList tail;};
TMP_labelList TMP_LabelList(TMP_label head, TMP_labelList tail);



/*Temp*/
typedef struct TMP_temp_ *TMP_temp;
TMP_temp TMP_newtemp();
typedef struct TMP_tempList_ *TMP_tempList;
struct TMP_tempList_ { TMP_temp head; TMP_tempList tail;};
typedef struct TMP_map_ *TMP_map;

TMP_map TMP_name();
TMP_map newMap(TB_table table, TMP_map under);
TMP_map TMP_empty();
TMP_map TMP_layerMap(TMP_map overM, TMP_map underM);
void TMP_enter(TMP_map map, TMP_temp te, string str);
string TMP_look(TMP_map map, TMP_temp te);
TMP_tempList TMP_TempList(TMP_temp head, TMP_tempList tail);

#endif
