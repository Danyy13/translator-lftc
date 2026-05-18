#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "../analizor-domeniu/domain.h"
#include "../analizor-lexical/utils.h"

// #define DEBUG

#define MISSING_SEMICOLON_MESSAGE "Expected ';'"
#define MISSING_RACC_MESSAGE "Expected '}'"
#define MISSING_LACC_MESSAGE "Expected '{'"

Token *iteratorToken; // the iterator in the tokens list
Token *consumedToken; // the last consumed token

Symbol *owner = NULL;

// pre-declaration of functions that need it
bool stm();
bool expr();

void printTokenErrorAndExit(const char *fmt, ...) {
    fprintf(stderr, "Error in line %d: ", iteratorToken->line);
    
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

bool consume(int code) {
#ifdef DEBUG
    printf("consume(%s)", getAtomName(code));
#endif

    if(iteratorToken->code == code) {
        consumedToken = iteratorToken;
        iteratorToken = iteratorToken->next;
    
    #ifdef DEBUG
        printf(" => consumed\n");
    #endif

        return true;
    }

#ifdef DEBUG
    printf(" => found %s from line %d\n", getAtomName(iteratorToken->code), iteratorToken->line);
#endif

    return false;
}

bool typeBase(Type *type) {
#ifdef DEBUG
    printf("# typeBase\n");
#endif

    if(consume(TYPE_INT)) {
        type->typeBase = TB_INT;   
        return true;
    }
    if(consume(TYPE_DOUBLE)) {
        type->typeBase = TB_DOUBLE;   
        return true;
    }
    if(consume(TYPE_CHAR)) {
        type->typeBase = TB_CHAR;   
        return true;
    }
    
    if(consume(STRUCT)) {
        if(consume(ID)) {
            Token *tokenName = consumedToken;

            type->typeBase = TB_STRUCT;
            type->symbol = findSymbol(tokenName->value.text);

            if(!type->symbol) {
                printTokenErrorAndExit("Structura nedefinita: %s", tokenName->value.text);
            }

            return true;
        }
    }

    return false;
}

bool arrayDecl(Type *type) {
#ifdef DEBUG
    printf("# arrayDecl\n");
#endif

    if(consume(LBRACKET)) {
        if(consume(INT)) { // optional INT
            Token *tokenSize = consumedToken;
            type->arraySize = tokenSize->value.intValue;
        } else {
            type->arraySize = 0;
        }

        if(consume(RBRACKET)) {
            return true;
        }
        printTokenErrorAndExit("Expected ']' after array declaration");
    }
    
    return false;
}

bool varDef() {
#ifdef DEBUG
    printf("# varDef\n");
#endif

    Type type;

    if(typeBase(&type)) {
        if(consume(ID)) {
            Token *tokenName = consumedToken;

            if(arrayDecl(&type)) {
                if(type.arraySize == 0) printTokenErrorAndExit("A vector variable must have a specified dimension");
            } else {
                type.arraySize = -1; // daca array da fail, arraySize era garbage value
            }

            if(consume(SEMICOLON)) {
                Symbol *var = findSymbolInDomain(symbolTable, tokenName->value.text);
                if(var) {
                    printTokenErrorAndExit("Symbol redefinition %s", tokenName->value.text);
                }
                var = newSymbol(tokenName->value.text, SK_VAR);
                var->type = type;
                var->owner = owner;
                addSymbolToDomain(symbolTable, var);

                if(owner) {
                    switch(owner->symbolKind) {
                        case SK_FN:
                            var->varIndex = symbolsLen(owner->function.locals);
                            addSymbolToList(&owner->function.locals, dupSymbol(var));
                            break;
                        case SK_STRUCT:
                            var->varIndex = typeSize(&owner->type);
                            addSymbolToList(&owner->structMembers, dupSymbol(var));
                            break;
                        case SK_VAR:
                        case SK_PARAM:
                            break;
                    }
                } else {
                    // printf("Type\tarraySize: %d\tsymbol", type.arraySize);
                    // showSymbol(type.symbol);
                    // printf("\ttypeBase: ");
                    // showNamedType(&type, "ok");
                    // printf("\tsize: %d\n", typeSize(&type));
                    
                    var->varMem = safeMalloc(typeSize(&type));
                }

                return true;
            }
            printTokenErrorAndExit("Missing ';' after variable declaration");
        }
        printTokenErrorAndExit("Missing or invalid variable name or function identifier");
    }

    return false;
}

bool structDef() {
#ifdef DEBUG
    printf("# StructDef\n");
#endif

    Token *start = iteratorToken;

    if(consume(STRUCT)) {
        if(consume(ID)) {
            Token *tokenName = consumedToken;

            if(consume(LACC)) {
                Symbol *symbol = findSymbolInDomain(symbolTable, tokenName->value.text);
                if(symbol) {
                    printTokenErrorAndExit("Symbol redefinition: %s", tokenName->value.text);
                }
                symbol = addSymbolToDomain(symbolTable, newSymbol(tokenName->value.text, SK_STRUCT));
                symbol->type.typeBase = TB_STRUCT;
                symbol->type.symbol = symbol;
                symbol->type.arraySize = -1;

                pushDomain();
                owner = symbol;

                while(varDef()) { }
            
                if(consume(RACC)) {
                    if(consume(SEMICOLON)) {
                        owner = NULL;
                        dropDomain();

                        return true;
                    }
                    printTokenErrorAndExit("Missing ';' after struct declaration");
                }
                printTokenErrorAndExit("Missing '}' after struct declaration");
            }
        }
    }

    iteratorToken = start;
    return false;
}

bool stmCompound(bool newDomain) {
#ifdef DEBUG
    printf("# stmCompound\n");
#endif

    if(consume(LACC)) {
        if(newDomain) pushDomain();

        while(varDef() || stm()) { }

        if(consume(RACC)) {
        #ifdef DEBUG
            printf("# finished stmCompound as true\n");
        #endif

            if(newDomain) {
                dropDomain();
            }

            return true;
        }
        // printTokenErrorAndExit("Missing '}' at the end of statement");
    }

    return false;
}

bool stm() {
#ifdef DEBUG
    printf("# stm\n");
#endif    

    if(stmCompound(true)) { return true; }

    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr()) {
                if(consume(RPAR)) {
                    if(stm()) {
                        if(consume(ELSE)) { // optional
                            if(stm()) {
                                return true;
                            }
                            printTokenErrorAndExit("Invalid or missing expression after else keyword");
                        }

                        return true;
                    }
                }
                printTokenErrorAndExit("Missing ')' after if condition");
            }
        }
        printTokenErrorAndExit("Missing '(' after if keyword");
    }

    if(consume(WHILE)) {
        if(consume(LPAR)) {
            if(expr()) {
                if(consume(RPAR)) {
                    if(stm()) {
                        return true;
                    }
                    printTokenErrorAndExit("Invalid or missing expression after while declaration");
                }
                printTokenErrorAndExit("Missing ')' after while condition");
            }
        }
        printTokenErrorAndExit("Missing '(' after while keyword");
    }

    if(consume(RETURN)) {
        if(expr()) { }
        
        if(consume(SEMICOLON)) {
            return true;
        }
        printTokenErrorAndExit("Missing ';' after return expression");
    }

    if(expr()) {}
    if(consume(SEMICOLON)) {
        return true;
    }

#ifdef DEBUG
    printf("# stm ended with false\n");
#endif
    
    return false;
}

bool fnParam() {
#ifdef DEBUG
    printf("# fnParam\n");
#endif

    Type type;
    type.arraySize = -1;

    if(typeBase(&type)) {
        if(consume(ID)) {
            Token *tokenName = consumedToken;

            if(arrayDecl(&type)) {
                type.arraySize = 0;
            }

            Symbol *parameter = findSymbolInDomain(symbolTable, tokenName->value.text);
            if(parameter) {
                printTokenErrorAndExit("Symbol redefinition: %s", tokenName->value.text);
            }

            parameter = newSymbol(tokenName->value.text, SK_PARAM);
            parameter->type = type;
            parameter->owner = owner;
            parameter->paramIndex = symbolsLen(owner->function.params);
            addSymbolToDomain(symbolTable, parameter);

            addSymbolToList(&owner->function.params, dupSymbol(parameter));

            return true;
        }
        printTokenErrorAndExit("Missing parameter name");
    }
    
    return false;
}

bool fnDef() {
#ifdef DEBUG
    printf("# fnDef\n");
#endif

    Token *start = iteratorToken;
    Type type;
    type.arraySize = -1;

    bool isVoid = false;

    if(typeBase(&type) || (isVoid = consume(VOID))) {
        if(isVoid) type.typeBase = TB_VOID;

        if(consume(ID)) {
            Token *tokenName = consumedToken;

            if(consume(LPAR)) {
                Symbol *function = findSymbolInDomain(symbolTable, tokenName->value.text);
                if(function) {
                    printTokenErrorAndExit("Symbol redefinition: %s", tokenName->value.text);
                }

                function = newSymbol(tokenName->value.text, SK_FN);
                function->type = type;
                addSymbolToDomain(symbolTable, function);

                owner = function;
                pushDomain();

                if(fnParam()) { // parameters are optional
                    while(consume(COMMA)) {
                        if(!fnParam()) printTokenErrorAndExit("Expected parameter after ',' in function header");
                    }
                }

                if(consume(RPAR)) {
                    if(stmCompound(false)) {
                        dropDomain();
                        owner = NULL;
                        
                        return true;
                    }
                    printTokenErrorAndExit("Expected compound statement");
                }
                printTokenErrorAndExit("Missing ')'");
            }

            // return false; // could be a variable or struct, so don't throw error
        }
    }

    iteratorToken = start;
    return false;
}

bool exprPrimary() {
#ifdef DEBUG
    printf("# exprPrimary\n");
#endif
    
    
    if(consume(ID)) {
        if(consume(LPAR)) { // optional
            if(expr()) {
                while(consume(COMMA)) {
                    if(!expr()) printTokenErrorAndExit("Expected expression after ',' in function call");
                }
            }

            if(consume(RPAR)) {
                return true;
            } else {
                printTokenErrorAndExit("Expected ')'");
            }
        }

        return true;
    }

    if(consume(INT)) { return true; }
    if(consume(DOUBLE)) { return true; }
    if(consume(CHAR)) { return true; }
    if(consume(STRING)) { return true; }

    if(consume(LPAR)) {
        if(expr()) {
            if(consume(RPAR)) {
                return true;
            }
            printTokenErrorAndExit("Missing ')'");
        }
        // Can't check for errors as cast also begins with LPAR so we are not in a point of no return
    }

    
    return false;
}

bool exprPostfixPrim() {
#ifdef DEBUG
    printf("# exprPostfixPrim\n");
#endif

    if(consume(LBRACKET)) {
        if(expr()) {
            if(consume(RBRACKET)) {
                if(exprPostfixPrim()) {
                    return true;
                }
                printTokenErrorAndExit("Invalid or missing expression");
            }
            printTokenErrorAndExit("Missing ']'");
        }
        printTokenErrorAndExit("Invalid or missing expression after '['");
    }

    if(consume(DOT)) {
        if(consume(ID)) {
            if(exprPostfixPrim()) {
                return true;
            }
        } else printTokenErrorAndExit("Missing name");
    }

    return true;
}

bool exprPostfix() {
#ifdef DEBUG
    printf("# exprPostfix\n");
#endif

    

    if(exprPrimary()) {
        if(exprPostfixPrim()) {
            return true;
        }
    }

    
    return false;
}

bool exprUnary() {
#ifdef DEBUG
    printf("# exprUnary\n");
#endif

    

    if(consume(SUB)) {
        if(exprUnary()) {
            return true;
        }
        printTokenErrorAndExit("Invalid or missing expression after unary operator '-'");
    }

    if(consume(NOT)) {
        if(exprUnary()) {
            return true;
        }
        printTokenErrorAndExit("Invalid or missing expression after unary operator '!'");
    }

    if(exprPostfix()) {
        return true;
    }

    
    return false;
}

bool exprCast() {
#ifdef DEBUG
    printf("# exprCast\n");
#endif

    if(consume(LPAR)) {
        Type type;

        if(typeBase(&type)) {
            if(arrayDecl(&type)) { }

            if(consume(RPAR)) {
                if(exprCast()) {
                    return true;
                }
                printTokenErrorAndExit("Invalid or missing expression after cast");
            }
            printTokenErrorAndExit("Missing ')' after cast");
        }
        printTokenErrorAndExit("Invalid or missing type in cast");
    }

    if(exprUnary()) {
        return true;
    }

    return false;
}

bool exprMulPrim() {
#ifdef DEBUG
    printf("# exprMulPrim\n");
#endif

    if(consume(MUL)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '*'");
    }

    if(consume(DIV)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '/'");
    }

    return true;
}

bool exprMul() {
#ifdef DEBUG
    printf("# exprMul\n");
#endif

    if(exprCast()) {
        if(exprMulPrim()) {
            return true;
        }
    }
    
    return false;
}

bool exprAddPrim() {
#ifdef DEBUG
    printf("# exprAddPrim\n");
#endif

    if(consume(ADD)) {
        if(exprMul()) {
            if(exprAddPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '+'");
    }

    if(consume(SUB)) {
        if(exprMul()) {
            if(exprAddPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '-'");
    }    

    return true;
}

bool exprAdd() {
#ifdef DEBUG
    printf("# exprAdd\n");
#endif

    if(exprMul()) {
        if(exprAddPrim()) {
            return true;
        }
    }

    return false;
}

bool exprRelPrim() {
#ifdef DEBUG
    printf("# exprRelPrim\n");
#endif

    if(consume(LESS)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '<'");
    }

    if(consume(LESSEQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '<='");
    }

    if(consume(GREATER)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '>'");
    }

    if(consume(GREATEREQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '>='");
    }

    return true;
}

bool exprRel() {
#ifdef DEBUG
    printf("# exprRel\n");
#endif

    if(exprAdd()) {
        if(exprRelPrim()) {
            return true;
        }
    }
    
    return false;
}

bool exprEqPrim() {
#ifdef DEBUG
    printf("# exprEqPrim\n");
#endif

    if(consume(EQUAL)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '=='");
    }

    if(consume(NOTEQ)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '!='");
    }

    return true;
}

bool exprEq() {
#ifdef DEBUG
    printf("# exprEq\n");
#endif

    if(exprRel()) {
        if(exprEqPrim()) {
            return true;
        }
    }
    
    return false;
}

bool exprAndPrim() {
#ifdef DEBUG
    printf("# exprAndPrim\n");
#endif

    if(consume(AND)) {
        if(exprEq()) {
            if(exprAndPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '&&'");
    }

    return true;
}

bool exprAnd() {
#ifdef DEBUG
    printf("# exprAnd\n");
#endif

    if(exprEq()) {
        if(exprAndPrim()) {
            return true;
        }
    }
    
    return false;
}

bool exprOrPrim() {
#ifdef DEBUG
    printf("# exprOrPrim\n");
#endif

    if(consume(OR)) {
        if(exprAnd()) {
            if(exprOrPrim()) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '||'");
    }

    return true;
}

bool exprOr() {
#ifdef DEBUG
    printf("# exprOr\n");
#endif

    // Recursivitate stanga
    if(exprAnd()) {
        if(exprOrPrim()) {
            return true;
        }
    }
    
    return false;
}

bool exprAssign() {
#ifdef DEBUG
    printf("# exprAssign\n");
#endif

    Token *start = iteratorToken;

    if(exprUnary()) {
        if(consume(ASSIGN)) {
            if(exprAssign()) {
                return true;
            }
            printTokenErrorAndExit("Invalid or missing expression after '='");
        }
    }

    iteratorToken = start;

    if(exprOr()) {
        return true;
    }

    iteratorToken = start;
    return false;
}

bool expr() {
#ifdef DEBUG
    printf("# expr\n");
#endif

    if(exprAssign()) {
        return true;
    }

    return false;
}

bool unit() {
    for(;;) {
        if(structDef()) {}
        else if(fnDef()) {}
        else if(varDef()) {}
        else break;
    }

    if(consume(END)) {
        return true;
    }

    return false;
}

void parse(Token *tokens) {
    iteratorToken = tokens;
    if(!unit()) printTokenErrorAndExit("Syntax Error");
}