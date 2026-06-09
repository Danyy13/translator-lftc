#include "gc.h"

void insertConvIfNeeded(Instruction *before,Type *srcType,Type *dstType) {
	switch(srcType->typeBase) {
		case TB_INT:
			switch(dstType->typeBase) {
				case TB_DOUBLE:
					insertInstruction(before,OP_CONV_I_F);
					break;
				default: break;
				}
			break;
		case TB_DOUBLE:
			switch(dstType->typeBase) {
				case TB_INT:
					insertInstruction(before,OP_CONV_F_I);
					break;
					default: break;
				}
			break;
		default: break;
	}
}

void addRVal(Instruction **code,bool lval,Type *type){
	if(!lval)return;
	switch(type->typeBase) {
		case TB_INT:
			addInstruction(code,OP_LOAD_I);
			break;
		case TB_DOUBLE:
			addInstruction(code,OP_LOAD_F);
			break;
		default: break;
	}
}
