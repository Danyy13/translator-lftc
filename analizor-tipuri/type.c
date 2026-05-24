#include <string.h>
#include "type.h"

bool canBeScalar(Ret *ret) {
    Type *type = &ret->type;

    if(type->arraySize >= 0) return false; // array cannot be scalar
    if(type->typeBase == TB_VOID) return false; // void cannot be scalar

    return true;
}

bool convertsTo(Type *src, Type *dst) {
    // the pointers (arrays) can be converted one to another, but in nothing else
    if(src->arraySize >= 0) {
        if(dst->arraySize >= 0) return true;
        return false;
    }
    if(dst->arraySize >= 0) return false; // dst is array but src is not

    switch(src->typeBase) {
        // int, double, char can freely convert
        case TB_INT:
        case TB_DOUBLE:
        case TB_CHAR:
            switch(dst->typeBase) {
                case TB_INT:
                case TB_CHAR:
                case TB_DOUBLE:
                    return true;
                default:
                    return false;
            }
        
        case TB_STRUCT:
            if(dst->typeBase == TB_STRUCT && src->symbol == dst->symbol) return true;
            return false;
        default:
            return false;
    }
}

bool resultsArithmeticalType(Type *op1, Type *op2, Type *result) {
    // there are no arithmetic operations with pointers
    if(op1->arraySize >= 0 || op2->arraySize >= 0) return false;

	// the result of an arithmetic operation cannot be pointer or struct
    result->symbol = NULL;
    result->arraySize = -1;

    switch(op1->typeBase) {
        case TB_INT:
            switch(op2->typeBase) {
                case TB_INT:
                case TB_CHAR:
                    result->typeBase = TB_INT;
                    return true;
                case TB_DOUBLE:
                    result->typeBase = TB_DOUBLE;
                    return true;
                default:
                    return false;
            }
        case TB_DOUBLE:
            switch(op2->typeBase) {
                case TB_INT:
                case TB_DOUBLE:
                case TB_CHAR:
                    result->typeBase = TB_DOUBLE;
                    return true;
                default:
                    return false;
            }
        case TB_CHAR:
            switch(op2->typeBase) {
                case TB_INT:
                case TB_DOUBLE:
                case TB_CHAR:
                    result->typeBase = op2->typeBase;
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

Symbol *findSymbolInList(Symbol *list, const char *name) {
    for(Symbol *symbol=list; symbol; symbol=symbol->next) {
        if(!strcmp(symbol->name, name)) return symbol;
    }
    
    return NULL;
}