#pragma once

// code generation

#include "../analizor-tipuri/type.h"
#include "../masina-virtuala/vm.h"

// inserts after the specified instruction a conversion instruction
// only if necessary
void insertConvIfNeeded(Instruction *before, Type *srcType, Type *dstType);

// if lval is true, generates an rval from the current value from stack
void addRVal(Instruction **code, bool lval, Type *type);
