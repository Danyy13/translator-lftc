#include <stdio.h>

#include "../analizor-lexical/utils.h"
#include "../analizor-domeniu/domain.h"
#include "vm.h"

Instruction *addInstruction(Instruction **list,Opcode op){
	Instruction *i=(Instruction*)safeAlloc(sizeof(Instruction));
	i->op=op;
	i->next=NULL;
	if(*list){
		Instruction *p=*list;
		while(p->next)p=p->next;
		p->next=i;
		}else{
		*list=i;
		}
	return i;
	}

Instruction *addInstructionWithInt(Instruction **list,Opcode op,int argVal){
	Instruction *i=addInstruction(list,op);
	i->arg.i=argVal;
	return i;
	}

Instruction *addInstructionWithDouble(Instruction **list,Opcode op,double argVal){
	Instruction *i=addInstruction(list,op);
	i->arg.f=argVal;
	return i;
	}

Val stack[10000];		// the stack
Val *SP=stack-1;		// Stack pointer - the stack's top - points to the value from the top of the stack
Val *FP=NULL;		// the initial value doesn't matter

void pushv(Val v){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	*++SP=v;
	}

Val popv(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return *SP--;
	}

void pushi(int i){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->i=i;
	}

int popi(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->i;
	}

void pushp(void *p){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->p=p;
	}

void *popp(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->p;
	}

void put_i(){
	printf("=> %d",popi());
	}

void vmInit(){
	Symbol *fn=addExtFn("put_i",put_i,(Type){TB_VOID,NULL,-1});
	addFnParam(fn,"i",(Type){TB_INT,NULL,-1});
	}

void run(Instruction *IP){
	Val v;
	int iArg,iTop,iBefore;
	void(*extFnPtr)();
	for(;;){
		// shows the index of the current instruction and the number of values from stack
		printf("%p/%d\t",IP,(int)(SP-stack+1));
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
				printf("FPLOAD\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
				IP=IP->next;
				break;
			case OP_FPSTORE:
				v=popv();
				FP[IP->arg.i]=v;
				printf("FPSTORE\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
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
			default:err("run: instructiune neimplementata: %d",IP->op);
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
	Instruction *code=NULL;
	addInstructionWithInt(&code,OP_PUSH_I,2);
	Instruction *callPos=addInstruction(&code,OP_CALL);
	addInstruction(&code,OP_HALT);
	callPos->arg.instruction=addInstructionWithInt(&code,OP_ENTER,1);
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
	if(!s)err("undefined: put_i");
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
