#include "types.h"
#include "util.h"

static struct TP_tp_ tpint = { TP_int };
static struct TP_tp_ tpstring = { TP_string };
static struct TP_tp_ tpvoid = {TP_void};

TP_tp TP_Int(void) {
    return &tpint;
}
TP_tp TP_String(void) {
    return &tpstring;
}
TP_tp TP_Void(){
	return &tpvoid;
}
TP_tp TP_Array(TP_tp tp) {
    TP_tp ptr = checked_malloc(sizeof(*ptr));
    ptr->kind = TP_array;
    ptr->u.array = tp;
    return ptr;
}
