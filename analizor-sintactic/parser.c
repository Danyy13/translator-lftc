#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "../analizor-domeniu/domain.h"
#include "../analizor-lexical/utils.h"
#include "../analizor-tipuri/type.h"

// #define DEBUG

Token *iteratorToken; // the iterator in the tokens list
Token *consumedToken; // the last consumed token

Symbol *owner = NULL;

// pre-declaration of functions that need it
bool stm();
bool expr(Ret *ret);
bool exprMulPrim(Ret *ret);
bool exprAddPrim(Ret *ret);
bool exprRelPrim(Ret *ret);

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
        printTokenErrorAndExit("Missing or invalid struct identifier");
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
        printTokenErrorAndExit("Missing or invalid variable name or function name or missing '{' from struct definition");
    }

    // Check if type name is missing
    // Token *start = iteratorToken;
    // if(consume(ID)) {
    //     if(consume(SEMICOLON)) {
    //         printTokenErrorAndExit("Missing or invalid type name in variable declaration");
    //     }
    // }

    // iteratorToken = start;
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
        // printTokenErrorAndExit("Missing or invalid struct name");
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

    Ret condition, expression;

    if(stmCompound(true)) { return true; }

    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr(&condition)) {
                if(!canBeScalar(&condition)) printTokenErrorAndExit("The if condition must be a scalar value"); // tipuri

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
            if(expr(&condition)) {
                if(!canBeScalar(&condition)) printTokenErrorAndExit("The while condition must be a scalar value"); // tipuri

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
        if(expr(&expression)) {
            if(owner->type.typeBase == TB_VOID) printTokenErrorAndExit("A void function cannot return a value");
            if(!canBeScalar(&expression)) printTokenErrorAndExit("The return value must be a scalar value");
            if(!convertsTo(&expression.type, &owner->type)) printTokenErrorAndExit("Cannot convert the return expression type to the function return type");
        } else {
            if(owner->type.typeBase != TB_VOID) printTokenErrorAndExit("A non-void function must return a value");
        }
        
        if(consume(SEMICOLON)) {
            return true;
        }
        printTokenErrorAndExit("Missing ';' after return expression");
    }

    if(expr(&expression)) {}
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
    // printTokenErrorAndExit("Missing or invalid type name in parameter declaration");
    
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
                        if(!fnParam()) printTokenErrorAndExit("Missing or invalid parameter after ',' in function header");
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

bool exprPrimary(Ret *ret) {
#ifdef DEBUG
    printf("# exprPrimary\n");
#endif
    
    if(consume(ID)) {
        Token *tokenName = consumedToken;
        Symbol *symbol = findSymbol(tokenName->value.text);
        if(!symbol) printTokenErrorAndExit("Undefined identificator: \"%s\"", tokenName->value.text);

        if(consume(LPAR)) { // optional
            if(symbol->symbolKind != SK_FN) printTokenErrorAndExit("Only a function can be called");
            Ret retArgument;
            Symbol *param = symbol->function.params;

            if(expr(&retArgument)) {
                if(!param) printTokenErrorAndExit("Too many arguments in function call");
                if(!convertsTo(&retArgument.type, &param->type)) printTokenErrorAndExit("In function call, cannot convert to the argument type to the parameter type");
                param = param->next;

                while(consume(COMMA)) {
                    if(!expr(&retArgument)) printTokenErrorAndExit("Expected expression after ',' in function call");
                    //expr consumed
                    if(!param) printTokenErrorAndExit("Too many arguments in function call");
                    if(!convertsTo(&retArgument.type, &param->type)) printTokenErrorAndExit("In function call, cannot convert to the argument type to the parameter type");
                    param = param->next;
                }
            }

            if(consume(RPAR)) {
                if(param) printErrorAndExit("Too few arguments in function call");
                *ret = (Ret){symbol->type, false, true};

                return true;
            }
            printTokenErrorAndExit("Expected ')' after function call");
        }

        if(symbol->symbolKind == SK_FN) printTokenErrorAndExit("A function can only be called");
        *ret = (Ret){symbol->type, true, symbol->type.arraySize >= 0};

        return true;
    }

    if(consume(INT)) {
        *ret = (Ret){{TB_INT, NULL, -1}, false, true};
        return true;
    }

    if(consume(DOUBLE)) {
        *ret = (Ret){{TB_DOUBLE, NULL, -1}, false, true};
        return true;
    }

    if(consume(CHAR)) {
        *ret = (Ret){{TB_CHAR, NULL, -1}, false, true};
        return true;
    }

    if(consume(STRING)) {
        *ret = (Ret){{TB_CHAR, NULL, -1}, false, true};
        return true;
    }

    if(consume(LPAR)) {
        if(expr(ret)) {
            if(consume(RPAR)) {
                return true;
            }
            printTokenErrorAndExit("Missing ')'");
        }
        // Can't check for errors as cast also begins with LPAR so we are not in a point of no return
    }

    return false;
}

bool exprPostfixPrim(Ret *ret) {
#ifdef DEBUG
    printf("# exprPostfixPrim\n");
#endif

    if(consume(LBRACKET)) {
        Ret index;

        if(expr(&index)) {
            if(consume(RBRACKET)) {
                if(ret->type.arraySize < 0) printTokenErrorAndExit("Only an array can be indexed");
                
                Type typeInt = {TB_INT, NULL, -1};
                if(!convertsTo(&index.type, &typeInt)) printTokenErrorAndExit("The index is not convertible to int");
                ret->type.arraySize = -1;
                ret->isleftValue = true;
                ret->isConstant = false;

                if(exprPostfixPrim(ret)) {
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
            Token *tokenName = consumedToken;

            if(ret->type.typeBase != TB_STRUCT) printTokenErrorAndExit("A field can only be selected from a struct");
            
            Symbol *symbol = findSymbolInList(ret->type.symbol->structMembers, tokenName->value.text);
            if(!symbol) printTokenErrorAndExit("The structure %s does not have a field \"%s\"", ret->type.symbol->name, tokenName->value.text);
            *ret=(Ret){symbol->type, true, symbol->type.arraySize >= 0};

            if(exprPostfixPrim(ret)) {
                return true;
            }
        } else printTokenErrorAndExit("Missing name");
    }

    return true;
}

bool exprPostfix(Ret *ret) {
#ifdef DEBUG
    printf("# exprPostfix\n");
#endif

    if(exprPrimary(ret)) {
        if(exprPostfixPrim(ret)) {
            return true;
        }
    }

    return false;
}

bool exprUnary(Ret *ret) {
#ifdef DEBUG
    printf("# exprUnary\n");
#endif

    if(consume(SUB)) {
        if(exprUnary(ret)) {
            if(!canBeScalar(ret)) printTokenErrorAndExit("Unary '-' must have scalar operand");

            return true;
        }
        printTokenErrorAndExit("Invalid or missing expression after unary operator '-'");
    }

    if(consume(NOT)) {
        if(exprUnary(ret)) {
            if(!canBeScalar(ret)) printTokenErrorAndExit("Unary '!' must have scalar operand");

            return true;
        }
        printTokenErrorAndExit("Invalid or missing expression after unary operator '!'");
    }

    ret->isleftValue = false;
    ret->isConstant = true;

    if(exprPostfix(ret)) {
        return true;
    }
    
    return false;
}

bool exprCast(Ret *ret) {
#ifdef DEBUG
    printf("# exprCast\n");
#endif

    if(consume(LPAR)) {
        Type type;
        Ret op;

        if(typeBase(&type)) {
            if(arrayDecl(&type)) { }

            if(consume(RPAR)) {
                if(exprCast(&op)) {
                    if(type.typeBase == TB_STRUCT) printTokenErrorAndExit("Cannot convert to a struct type");
                    if(op.type.typeBase == TB_STRUCT) printTokenErrorAndExit("Cannot convert a struct");
                    if(op.type.arraySize >= 0 && type.arraySize < 0) printTokenErrorAndExit("An array can be converted only to another array");
                    if(op.type.arraySize < 0 && type.arraySize >= 0) printTokenErrorAndExit("A scalar can only be converted to another scalar");
                    *ret = (Ret){type, false, true};

                    return true;
                }
                printTokenErrorAndExit("Invalid or missing expression after cast");
            }
            printTokenErrorAndExit("Missing ')' after cast");
        }
        printTokenErrorAndExit("Invalid or missing type in cast");
    }

    if(exprUnary(ret)) {
        return true;
    }

    return false;
}

bool handleExprMulPrim(Ret *ret, AtomCode atomCode) {
    char operand = 0;

    switch(atomCode) {
        case MUL:
            operand = '*';
            break;
        case DIV:
            operand = '/';
            break;
        default:
            break;
    }

    Ret right;

    if(exprCast(&right)) {
        Type destination;
        if(!resultsArithmeticalType(&ret->type, &right.type, &destination)) printTokenErrorAndExit("Invalid operand type for %c", operand);
        *ret = (Ret){destination, false, true};

        if(exprMulPrim(ret)) {
            return true;
        }
    }
    printTokenErrorAndExit("Invalid or missing expression after '+'");

    return true;
}

bool exprMulPrim(Ret *ret) {
#ifdef DEBUG
    printf("# exprMulPrim\n");
#endif

    if(consume(MUL)) {
        handleExprMulPrim(ret, MUL);
    }

    if(consume(DIV)) {
        handleExprMulPrim(ret, DIV);
    }

    return true;
}

bool exprMul(Ret *ret) {
#ifdef DEBUG
    printf("# exprMul\n");
#endif

    if(exprCast(ret)) {
        if(exprMulPrim(ret)) {
            return true;
        }
    }
    
    return false;
}

bool handleExprAddPrim(Ret *ret, AtomCode atomCode) {
    char operand = 0;

    switch(atomCode) {
        case ADD:
            operand = '+';
            break;
        case SUB:
            operand = '-';
            break;
        default:
            break;
    }

    Ret right;

    if(exprMul(&right)) {
        Type destination;
        if(!resultsArithmeticalType(&ret->type, &right.type, &destination)) printTokenErrorAndExit("Invalid operand type for %c", operand);
        *ret = (Ret){destination, false, true};

        if(exprAddPrim(ret)) {
            return true;
        }
    }
    printTokenErrorAndExit("Invalid or missing expression after '+'");

    return true;
}

bool exprAddPrim(Ret *ret) {
#ifdef DEBUG
    printf("# exprAddPrim\n");
#endif

    if(consume(ADD)) {
        handleExprAddPrim(ret, ADD);
    }

    if(consume(SUB)) {
        handleExprAddPrim(ret, SUB);
    }    

    return true;
}

bool exprAdd(Ret *ret) {
#ifdef DEBUG
    printf("# exprAdd\n");
#endif

    if(exprMul(ret)) {
        if(exprAddPrim(ret)) {
            return true;
        }
    }

    return false;
}

bool handleRelPrim(Ret *ret, AtomCode atomCode) {
    char operand[3];

    switch(atomCode) {
        case LESS:
            strncpy(operand, "<", 2);
            break;
        case LESSEQ:
            strncpy(operand, "<=", 3);
            break;
        case GREATER:
            strncpy(operand, ">", 2);
            break;
        case GREATEREQ:
            strncpy(operand, ">=", 3);
            break;
        default:
            break;
    }

    Ret right;

    if(exprAdd(&right)) {
        Type destination;
        if(!resultsArithmeticalType(&ret->type, &right.type, &destination)) printTokenErrorAndExit("Invalid operand type for %s", operand);
        *ret = (Ret){{TB_INT, NULL, -1}, false, true};

        if(exprRelPrim(ret)) {
            return true;
        }
    }
    printTokenErrorAndExit("Invalid or missing expression after '%s'", operand);

    return true;
}

bool exprRelPrim(Ret *ret) {
#ifdef DEBUG
    printf("# exprRelPrim\n");
#endif

    if(consume(LESS)) {
        handleRelPrim(ret, LESS);
    }

    if(consume(LESSEQ)) {
        handleRelPrim(ret, LESSEQ);
    }

    if(consume(GREATER)) {
        handleRelPrim(ret, GREATER);
    }

    if(consume(GREATEREQ)) {
        handleRelPrim(ret, GREATEREQ);
    }

    return true;
}

bool exprRel(Ret *ret) {
#ifdef DEBUG
    printf("# exprRel\n");
#endif

    if(exprAdd(ret)) {
        if(exprRelPrim(ret)) {
            return true;
        }
    }
    
    return false;
}

bool exprEqPrim(Ret *ret) {
#ifdef DEBUG
    printf("# exprEqPrim\n");
#endif

    if(consume(EQUAL)) {
        Ret right;

        if(exprRel(&right)) {
            Type destination;
            if(!resultsArithmeticalType(&ret->type, &right.type, &destination)) printTokenErrorAndExit("Invalid operand type for ==");
            *ret = (Ret){{TB_INT, NULL, -1}, false, true};

            if(exprEqPrim(ret)) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '=='");
    }

    if(consume(NOTEQ)) {
        Ret right;

        if(exprRel(&right)) {
            Type destination;
            if(!resultsArithmeticalType(&ret->type, &right.type, &destination)) printTokenErrorAndExit("Invalid operand type for !=");
            *ret = (Ret){{TB_INT, NULL, -1}, false, true};

            if(exprEqPrim(ret)) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '!='");
    }

    return true;
}

bool exprEq(Ret *ret) {
#ifdef DEBUG
    printf("# exprEq\n");
#endif

    if(exprRel(ret)) {
        if(exprEqPrim(ret)) {
            return true;
        }
    }
    
    return false;
}

bool exprAndPrim(Ret *ret) {
#ifdef DEBUG
    printf("# exprAndPrim\n");
#endif

    if(consume(AND)) {
        Ret right;

        if(exprEq(&right)) {
            Type destination;
            if(!resultsArithmeticalType(&ret->type, &right.type, &destination)) printTokenErrorAndExit("Invalid operand type for &&");
            *ret = (Ret){{TB_INT, NULL, -1}, false, true};

            if(exprAndPrim(ret)) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '&&'");
    }

    return true;
}

bool exprAnd(Ret *ret) {
#ifdef DEBUG
    printf("# exprAnd\n");
#endif

    if(exprEq(ret)) {
        if(exprAndPrim(ret)) {
            return true;
        }
    }
    
    return false;
}

bool exprOrPrim(Ret *ret) {
#ifdef DEBUG
    printf("# exprOrPrim\n");
#endif

    if(consume(OR)) {
        Ret right;

        if(exprAnd(&right)) {
            Type destination;
            if(!resultsArithmeticalType(&ret->type, &right.type, &destination)) printTokenErrorAndExit("Invalid operand type for ||");
            *ret = (Ret){{TB_INT, NULL, -1}, false, true};

            if(exprOrPrim(ret)) {
                return true;
            }
        }
        printTokenErrorAndExit("Invalid or missing expression after '||'");
    }

    return true;
}

bool exprOr(Ret *ret) {
#ifdef DEBUG
    printf("# exprOr\n");
#endif

    // Recursivitate stanga
    if(exprAnd(ret)) {
        if(exprOrPrim(ret)) {
            return true;
        }
    }
    
    return false;
}

bool exprAssign(Ret *ret) {
#ifdef DEBUG
    printf("# exprAssign\n");
#endif

    Token *start = iteratorToken;
    Ret destination;

    if(exprUnary(&destination)) {
        if(consume(ASSIGN)) {
            if(exprAssign(ret)) {
                if(!destination.isleftValue) printTokenErrorAndExit("The assign destination must be a left-value");
                if(destination.isConstant) printErrorAndExit("The assign destination cannot be constant");
                if(!canBeScalar(&destination)) printErrorAndExit("The assign destination must be scalar");
                if(!canBeScalar(ret)) printTokenErrorAndExit("The assign source must be scalar");
                if(!convertsTo(&ret->type, &destination.type)) printTokenErrorAndExit("The assign source cannot be converted to destination");
                ret->isleftValue = false;
                ret->isConstant = true;

                return true;
            }
            printTokenErrorAndExit("Invalid or missing expression after '='");
        }
    }

    iteratorToken = start;

    if(exprOr(ret)) {
        return true;
    }

    iteratorToken = start;
    return false;
}

bool expr(Ret *ret) {
#ifdef DEBUG
    printf("# expr\n");
#endif

    if(exprAssign(ret)) {
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