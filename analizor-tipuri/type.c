#include <string.h>
#include "type.h"

bool canBeScalar(Ret *ret) {
    Type *type = &ret->type;

    if(type->arraySize >= 0) return false; // array cannot be scalar
    if(type->typeBase == TB_VOID) return false; // void cannot be scalar

    return true;
}