#include <stdio.h>
#include <stdlib.h>

#include "../analizor-lexical/utils.h"
#include "../analizor-domeniu/domain.h"
#include "vm.h"

void put_d();
void pushf(double f);
double popf();

Instruction *addInstruction(Instruction **list,Opcode op){
	Instruction *instruction = (Instruction*)safeMalloc(sizeof(Instruction));
	instruction->op=op;
	instruction->next=NULL;
	if(*list){
		Instruction *p=*list;
		while(p->next)p=p->next;
		p->next=instruction;
		}else{
		*list=instruction;
		}
	return instruction;
	}

Instruction *insertInstruction(Instruction *before, int op) {
	Instruction *instruction = (Instruction*)safeMalloc(sizeof(Instruction));
	instruction->op = op;
	instruction->next = before->next;
	before->next = instruction;

	return instruction;
}

void deleteInstructionAfter(Instruction *instruction) {
	if(!instruction) return;
	
	for(Instruction *next = instruction->next,*i=next ; i ; i=next){
		next = i->next;
		free(i);
	}
	
	instruction->next = NULL;
}

Instruction *lastInstruction(Instruction *list) {
	if(list){
		while(list->next) list = list->next;
	}
	
	return list;
}

Instruction *addInstructionWithInt(Instruction **list,Opcode op,int argVal){
	Instruction *i=addInstruction(list,op);
	i->arg.i=argVal;
	return i;
	}

Instruction *addInstructionWithDouble(Instruction **list, Opcode op, double argVal){
	Instruction *i=addInstruction(list,op);
	i->arg.f=argVal;
	return i;
	}

#define MAX_STACK 10000
Val stack[MAX_STACK];		// the stack
Val *SP=stack-1;		// Stack pointer - the stack's top - points to the value from the top of the stack
Val *FP=NULL;		// the initial value doesn't matter

void pushv(Val v){
	if(SP+1==stack+10000)printErrorAndExit("trying to push into a full stack");
	*++SP=v;
	}

Val popv(){
	if(SP==stack-1)printErrorAndExit("trying to pop from empty stack");
	return *SP--;
	}

void pushi(int i){
	if(SP+1==stack+10000)printErrorAndExit("trying to push into a full stack");
	(++SP)->i=i;
	}

int popi(){
	if(SP==stack-1)printErrorAndExit("trying to pop from empty stack");
	return SP--->i;
	}

void pushp(void *p){
	if(SP+1==stack+10000)printErrorAndExit("trying to push into a full stack");
	(++SP)->p=p;
	}

void *popp(){
	if(SP==stack-1)printErrorAndExit("trying to pop from empty stack");
	return SP--->p;
	}

void put_i(){
	printf("=> %d",popi());
	}

void vmInit() {
	Symbol *puti = addExtFn("put_i", put_i, (Type){TB_VOID, NULL, -1});
	addFnParam(puti, "i", (Type){TB_INT, NULL, -1});

	Symbol *putd = addExtFn("put_d", put_d, (Type){TB_VOID, NULL, -1});
	addFnParam(putd, "i", (Type){TB_DOUBLE, NULL, -1});
}

void showInstructionList(Instruction *list) {
	Instruction *traverser = list;
	while(traverser != NULL) {
		printf("%d - %d - %.2lf\n", traverser->op, traverser->arg.i, traverser->arg.f);
		traverser = traverser->next;
	}
}

void run(Instruction *IP){
	Val v;
	int iArg, iTop, iBefore;
	double fTop, fBefore;
	void *pTop;
	void(*extFnPtr)();

	// showInstructionList(IP);
	
	for(;;){
		// shows the index of the current instruction and the number of values from stack
		printf("%p/%d\t", IP, (int)(SP - stack + 1));
		switch(IP->op){
			case OP_HALT:
				printf("HALT");
				return;
			case OP_PUSH_I:
				printf("PUSH.i\t%d",IP->arg.i);
				pushi(IP->arg.i);
				IP=IP->next;
				break;
			case OP_CALL:
				pushp(IP->next);
				printf("CALL\t%p",IP->arg.instruction);
				IP=IP->arg.instruction;
				break;
			case OP_CALL_EXT:
				extFnPtr=IP->arg.externFunctionPointer;
				printf("CALL_EXT\t%p\n",extFnPtr);
				extFnPtr();
				IP=IP->next;
				break;
			case OP_ENTER:
				pushp(FP);
				FP=SP;
				SP+=IP->arg.i;
				printf("ENTER\t%d",IP->arg.i);
				IP=IP->next;
				break;
			case OP_RET_VOID:
				iArg=IP->arg.i;
				printf("RET_VOID\t%d",iArg);
				IP=FP[-1].p;
				SP=FP-iArg-2;
				FP=FP[0].p;
				break;
			case OP_JMP:
				printf("JMP\t%p",IP->arg.instruction);
				IP=IP->arg.instruction;
				break;
			case OP_JF:
				iTop=popi();
				printf("JF\t%p\t// %d",IP->arg.instruction,iTop);
				IP=iTop ? IP->next : IP->arg.instruction;
				break;
			case OP_FPLOAD:
				v=FP[IP->arg.i];
				pushv(v);
				printf("FPLOAD\t%d\t// i:%d, f:%.2lf",IP->arg.i,v.i,v.f);
				IP=IP->next;
				break;
			case OP_FPSTORE:
				v=popv();
				FP[IP->arg.i]=v;
				printf("FPSTORE\t%d\t// i:%d, f:%.2lf",IP->arg.i,v.i,v.f);
				IP=IP->next;
				break;
			case OP_ADD_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore+iTop);
				printf("ADD.i\t// %d+%d -> %d",iBefore,iTop,iBefore+iTop);
				IP=IP->next;
				break;
			case OP_LESS_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore<iTop);
				printf("LESS.i\t// %d<%d -> %d",iBefore,iTop,iBefore<iTop);
				IP=IP->next;
				break;
			
			// added instructions for double function for vm
			case OP_PUSH_F:
				printf("PUSH.f\t%.2lf", IP->arg.f);
				pushf(IP->arg.f);
				IP=IP->next;
				break;
			case OP_ADD_F:
				fTop = popf();
				fBefore = popf();
				pushf(fBefore + fTop);
				printf("ADD.f\t// %.2lf+%.2lf -> %.2lf", fBefore, fTop, fBefore + fTop);
				IP=IP->next;
				break;
			case OP_LESS_F:
				fTop = popf();
				fBefore = popf();
				pushi(fBefore < fTop);
				printf("LESS.f\t// %.2lf<%.2lf -> %d", fBefore, fTop, fBefore < fTop);
				IP=IP->next;
				break;

			// added for code generation
			case OP_CONV_F_I:
				fTop=popf();
				pushi((int)fTop);
				printf("CONV.f.i\t// %g -> %d",fTop,(int)fTop);
				IP=IP->next;
				break;
			case OP_DROP:
				popv();
				printf("DROP");
				IP=IP->next;
				break;
			case OP_FPADDR_I:
				pTop=&FP[IP->arg.i].i;
				pushp(pTop);
				printf("FPADDR\t%d\t// %p",IP->arg.i,pTop);
				IP=IP->next;
				break;
			case OP_LOAD_I:
				pTop=popp();
				pushi(*(int*)pTop);
				printf("LOAD.i\t// *(int*)%p -> %d",pTop,*(int*)pTop);
				IP=IP->next;
				break;
			case OP_NOP:
				printf("NOP");
				IP=IP->next;
				break;
			case OP_RET:
				v=popv();
				iArg=IP->arg.i;
				printf("RET\t%d\t// i:%d, f:%g",iArg,v.i,v.f);
				IP=FP[-1].p;
				SP=FP-iArg-2;
				FP=FP[0].p;
				pushv(v);
				break;
			case OP_SUB_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore-iTop);
				printf("SUB.i\t// %d-%d -> %d",iBefore,iTop,iBefore-iTop);
				IP=IP->next;
				break;
			case OP_MUL_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore*iTop);
				printf("MUL.i\t// %d*%d -> %d",iBefore,iTop,iBefore*iTop);
				IP=IP->next;
				break;
			case OP_STORE_I:
				iTop=popi();
				v=popv();
				*(int*)v.p=iTop;
				pushi(iTop);
				printf("STORE.i\t// *(int*)%p=%d",v.p,iTop);
				IP=IP->next;
				break;

			default:printErrorAndExit("run: instructiune neimplementata: %d",IP->op);
			}
		putchar('\n');
		}
	}

/* The program implements the following AtomC source code:
f(2);
void f(int n){		// stack frame: n[-2] ret[-1] oldFP[0] i[1]
	int i=0;
	while(i<n){
		put_i(i);
		i=i+1;
		}
	}
*/

Instruction *genTestProgram(){
	Instruction *code = NULL;
	addInstructionWithInt(&code, OP_PUSH_I, 2);
	Instruction *callPos = addInstruction(&code,OP_CALL);
	addInstruction(&code, OP_HALT);
	callPos->arg.instruction = addInstructionWithInt(&code, OP_ENTER, 1);
	// int i=0;
	addInstructionWithInt(&code,OP_PUSH_I,0);
	addInstructionWithInt(&code,OP_FPSTORE,1);
	// while(i<n){
	Instruction *whilePos=addInstructionWithInt(&code,OP_FPLOAD,1);
	addInstructionWithInt(&code,OP_FPLOAD,-2);
	addInstruction(&code,OP_LESS_I);
	Instruction *jfAfter=addInstruction(&code,OP_JF);
	// put_i(i);
	addInstructionWithInt(&code,OP_FPLOAD,1);
	Symbol *s=findSymbol("put_i");
	if(!s)printErrorAndExit("undefined: put_i");
	addInstruction(&code,OP_CALL_EXT)->arg.externFunctionPointer=s->function.externFunctionPointer;
	// i=i+1;
	addInstructionWithInt(&code,OP_FPLOAD,1);
	addInstructionWithInt(&code,OP_PUSH_I,1);
	addInstruction(&code,OP_ADD_I);
	addInstructionWithInt(&code,OP_FPSTORE,1);
	// } ( the next iteration)
	addInstruction(&code,OP_JMP)->arg.instruction=whilePos;
	// returns from function
	jfAfter->arg.instruction=addInstructionWithInt(&code,OP_RET_VOID,1);
	return code;
}

// added instructions for double function for vm
void put_d() {
	printf("=> %.2lf", popf());
}

void pushf(double f) {
	if(SP+1 == stack + 10000) printErrorAndExit("trying to push into a full stack");
	(++SP)->f=f;
}

double popf() {
	if(SP == stack - 1) printErrorAndExit("trying to pop from empty stack");
	return SP--->f;
}

Instruction *genTestProgramDouble() {
	Instruction *code = NULL;
	addInstructionWithDouble(&code, OP_PUSH_F, 2.0);
	Instruction *callPos = addInstruction(&code, OP_CALL);
	addInstruction(&code, OP_HALT);
	callPos->arg.instruction = addInstructionWithInt(&code, OP_ENTER, 1);
	// int i=0.0;
	addInstructionWithDouble(&code, OP_PUSH_F, 0.0);
	addInstructionWithInt(&code, OP_FPSTORE, 1);
	// while(i<n){
	Instruction *whilePos = addInstructionWithInt(&code, OP_FPLOAD, 1); // fetch i
	addInstructionWithInt(&code, OP_FPLOAD, -2); // fetch parameter n
	addInstruction(&code, OP_LESS_F);
	Instruction *jfAfter=addInstruction(&code, OP_JF);
	// put_i(i);
	addInstructionWithInt(&code, OP_FPLOAD, 1); // fetch i
	Symbol *s = findSymbol("put_d");
	if(!s) printErrorAndExit("undefined: put_d");
	addInstruction(&code, OP_CALL_EXT)->arg.externFunctionPointer=s->function.externFunctionPointer;
	// i=i+0.5;
	addInstructionWithInt(&code, OP_FPLOAD, 1); // load i
	addInstructionWithDouble(&code, OP_PUSH_F, 0.5);
	addInstruction(&code, OP_ADD_F);
	addInstructionWithInt(&code, OP_FPSTORE, 1);
	// } ( the next iteration)
	addInstruction(&code, OP_JMP)->arg.instruction=whilePos;
	// returns from function
	jfAfter->arg.instruction=addInstructionWithInt(&code, OP_RET_VOID, 1);
	return code;
}