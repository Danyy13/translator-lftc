#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../analizor-lexical/utils.h"
#include "domain.h"

Domain *symbolTable = NULL;

int typeBaseSize(Type *type) {
    switch(type->typeBase) {
        case TB_INT: return sizeof(int);
        case TB_DOUBLE: return sizeof(double);
        case TB_CHAR: return sizeof(char);
        case TB_VOID: return 0;
        case TB_STRUCT:
            int size = 0;
            for(Symbol *traverser=type->symbol->structMembers; traverser!=NULL; traverser=traverser->next) {
                size += typeSize(&traverser->type);
            }
            return size;
        default:
            printErrorAndExit("Invalid Type parameter given to function. Please try TB_INT | TB_DOUBLE | TB_CHAR | TB_VOID | TB_STRUCT");
    }
}

int typeSize(Type *type) {
    if(type->arraySize < 0) return typeBaseSize(type);
    if(type->arraySize == 0) return sizeof(void *);
    return type->arraySize * typeBaseSize(type);
}

void freeSymbols(Symbol *list){
	for(Symbol *next;list;list=next){
		next=list->next;
		freeSymbol(list);
	}
}

Symbol *newSymbol(const char *name, SymbolKind symbolKind){
	Symbol *symbol = (Symbol*)safeMalloc(sizeof(Symbol));
	
    // sets all the fields to 0/NULL
	memset(symbol, 0, sizeof(Symbol));
	symbol->name = name;
	symbol->symbolKind = symbolKind;
	
    return symbol;
}

Symbol *dupSymbol(Symbol *symbol){
	Symbol *s=(Symbol*)safeMalloc(sizeof(Symbol));
	
    *s=*symbol;
	s->next=NULL;
	
    return s;
}

// s->next is already NULL from newSymbol
Symbol *addSymbolToList(Symbol **list, Symbol *symbol){
	Symbol *iterator=*list;

	if(iterator){
		while(iterator->next) iterator=iterator->next;
		
        iterator->next = symbol;
	} else {
		*list = symbol;
	}
	return symbol;
}

int symbolsLen(Symbol *list){
	int n=0;
	for(;list;list=list->next) n++;
	return n;
}

void freeSymbol(Symbol *symbol){
	switch(symbol->symbolKind){
		case SK_VAR:
			if(!symbol->owner) free(symbol->varMem);
			break;
		case SK_FN:
			freeSymbols(symbol->function.params);
			freeSymbols(symbol->function.locals);
			break;
		case SK_STRUCT:
			freeSymbols(symbol->structMembers);
			break;
		case SK_PARAM:
			break;
	}
	
    free(symbol);
}

Domain *pushDomain(){
	Domain *domain = (Domain*)safeMalloc(sizeof(Domain));
	
    domain->symbols = NULL;
	domain->parent = symbolTable;
	symbolTable = domain;
	
    return domain;
}

void dropDomain(){
	Domain *domain = symbolTable;

    symbolTable = domain->parent;
	freeSymbols(domain->symbols);

	free(domain);
}

void showNamedType(Type *t,const char *name){
	switch(t->typeBase){
		case TB_INT:printf("int");break;
		case TB_DOUBLE:printf("double");break;
		case TB_CHAR:printf("char");break;
		case TB_VOID:printf("void");break;
		default:		// TB_STRUCT
			printf("struct %s",t->symbol->name);
		}
	if(name)printf(" %s",name);
	if(t->arraySize==0)printf("[]");
	else if(t->arraySize>0)printf("[%d]",t->arraySize);
	}

void showSymbol(Symbol *s){
	switch(s->symbolKind){
			case SK_VAR:
				showNamedType(&s->type,s->name);
				if(s->owner){
					printf(";\t// size=%d, idx=%d\n",typeSize(&s->type),s->varIndex);
					}else{
					printf(";\t// size=%d, mem=%p\n",typeSize(&s->type),s->varMem);
					}
				break;
			case SK_PARAM:{
				showNamedType(&s->type,s->name);
				printf(" /*size=%d, idx=%d*/",typeSize(&s->type),s->paramIndex);
				}break;
			case SK_FN:{
				showNamedType(&s->type,s->name);
				printf("(");
				bool next=false;
				for(Symbol *param=s->function.params;param;param=param->next){
					if(next)printf(", ");
					showSymbol(param);
					next=true;
					}
				printf("){\n");
				for(Symbol *local=s->function.locals;local;local=local->next){
					printf("\t");
					showSymbol(local);
					}
				printf("\t}\n");
				}break;
			case SK_STRUCT:{
				printf("struct %s{\n",s->name);
				for(Symbol *m=s->structMembers;m;m=m->next){
					printf("\t");
					showSymbol(m);
					}
				printf("\t};\t// size=%d\n",typeSize(&s->type));
				}break;
		}
	}

void showDomain(Domain *d,const char *name){
	printf("// domain: %s\n",name);
	for(Symbol *s=d->symbols;s;s=s->next){
		showSymbol(s);
		}
	puts("\n");
	}

Symbol *findSymbolInDomain(Domain *d,const char *name){
	for(Symbol *s=d->symbols;s;s=s->next){
		if(!strcmp(s->name,name))return s;
		}
	return NULL;
	}

Symbol *findSymbol(const char *name){
	for(Domain *d=symbolTable;d;d=d->parent){
		Symbol *s=findSymbolInDomain(d,name);
		if(s)return s;
		}
	return NULL;
	}

Symbol *addSymbolToDomain(Domain *d,Symbol *s){
	return addSymbolToList(&d->symbols,s);
	}

Symbol *addExtFn(const char *name,void(*extFnPtr)(),Type ret){
	Symbol *fn=newSymbol(name,SK_FN);
	fn->function.externFunctionPointer=extFnPtr;
	fn->type=ret;
	addSymbolToDomain(symbolTable,fn);
	return fn;
	}

Symbol *addFnParam(Symbol *fn,const char *name,Type type){
	Symbol *param=newSymbol(name,SK_PARAM);
	param->type=type;
	param->paramIndex=symbolsLen(fn->function.params);
	addSymbolToList(&fn->function.params,dupSymbol(param));
	return param;
	}
