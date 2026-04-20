#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"

#define DEBUG

#define MISSING_SEMICOLON_MESSAGE "Expected ';'"
#define MISSING_RACC_MESSAGE "Expected '}'"
#define MISSING_LACC_MESSAGE "Expected '{'"

Token *iteratorToken; // the iterator in the tokens list
Token *consumedToken; // the last consumed token

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

bool typeBase() {
#ifdef DEBUG
    printf("# typeBase\n");
#endif

    Token *start = iteratorToken; // necessary for struct declaration

    if(consume(TYPE_INT)) return true;
    if(consume(TYPE_DOUBLE)) return true;
    if(consume(TYPE_CHAR)) return true;
    
    if(consume(STRUCT)) {
        if(consume(ID)) {
            return true;
        }
    }

    iteratorToken = start;
    return false;
}

bool arrayDecl() {
#ifdef DEBUG
    printf("# arrayDecl\n");
#endif

    Token *start = iteratorToken;

    if(consume(LBRACKET)) {
        if(consume(INT)) { } // optional INT

        if(consume(RBRACKET)) {
            return true;
        }
        printTokenErrorAndExit("Expected ']' after array declaration");
    }

    iteratorToken = start;
    return false;
}

bool varDef() {
#ifdef DEBUG
    printf("# varDef\n");
#endif

    Token *start = iteratorToken;

    if(typeBase()) {
        if(consume(ID)) {
            if(arrayDecl()) { }

            if(consume(SEMICOLON)) {
                return true;
            }
            printTokenErrorAndExit(MISSING_SEMICOLON_MESSAGE);
        }
    }

    iteratorToken = start;
    return false;
}

bool structDef() {
#ifdef DEBUG
    printf("# StructDef\n");
#endif

    Token *start = iteratorToken;

    if(consume(STRUCT)) {
        if(consume(ID)) {
            if(consume(LACC)) {
                while(varDef()) { }
            
                if(consume(RACC)) {
                    if(consume(SEMICOLON)) {
                        return true;
                    }
                    printTokenErrorAndExit(MISSING_SEMICOLON_MESSAGE);
                }
                printTokenErrorAndExit(MISSING_RACC_MESSAGE);
            }
            iteratorToken = start;
            return false; // could be variable declaration
        }
    }

    iteratorToken = start;
    return false;
}

bool stmCompound() {
#ifdef DEBUG
    printf("# stmCompound\n");
#endif

    Token *start = iteratorToken;

    if(consume(LACC)) {
        while(varDef() || stm()) { }

        if(consume(RACC)) {
        #ifdef DEBUG
            printf("# finished stmCompound as true\n");
        #endif
            return true;
        }
        printTokenErrorAndExit(MISSING_RACC_MESSAGE);
    }

    iteratorToken = start;
    return false;
}

bool stm() {
#ifdef DEBUG
    printf("# stm\n");
#endif

    Token *start = iteratorToken;

    if(stmCompound()) { return true; }

    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr()) {
                if(consume(RPAR)) {
                    if(stm()) {
                        if(consume(ELSE)) { // optional
                            if(stm()) { }
                        }

                        return true;
                    }
                }
                printTokenErrorAndExit("Missing ')'");
            }
        }
        printTokenErrorAndExit("Missing '('");
    }

    if(consume(WHILE)) {
        if(consume(LPAR)) {
            if(expr()) {
                if(consume(RPAR)) {
                    if(stm()) {
                        return true;
                    }
                }
                printTokenErrorAndExit("Missing ')'");
            }
        }
        printTokenErrorAndExit("Missing '('");
    }

    if(consume(RETURN)) {
        if(expr()) { }
        
        if(consume(SEMICOLON)) {
            return true;
        }
        printTokenErrorAndExit(MISSING_SEMICOLON_MESSAGE);
    }

    if(expr()) {}
    if(consume(SEMICOLON)) {
        return true;
    }

#ifdef DEBUG
    printf("# stm ended with false\n");
#endif

    iteratorToken = start;
    return false;
}

bool fnParam() {
#ifdef DEBUG
    printf("# fnParam\n");
#endif

    Token *start = iteratorToken;

    if(typeBase()) {
        if(consume(ID)) {
            if(arrayDecl()) { } // optional

            return true;
        }
        printTokenErrorAndExit("Missing parameter name");
    }

    iteratorToken = start;
    return false;
}

bool fnDef() {
#ifdef DEBUG
    printf("# fnDef\n");
#endif

    Token *start = iteratorToken;

    if(typeBase() || consume(VOID)) {
        if(consume(ID)) {
            if(consume(LPAR)) {
                while(fnParam()) { // parameters are optional
                    consume(COMMA); 
                    
                }

                if(consume(RPAR)) {
                    if(stmCompound()) {
                        return true;
                    }
                }
                printTokenErrorAndExit("Missing ')'");
            }

            iteratorToken = start;
            return false; // could be a variable or struct, so don't throw error
        }
    }

    iteratorToken = start;
    return false;
}

bool exprPrimary() {
#ifdef DEBUG
    printf("# exprPrimary\n");
#endif
    Token *start = iteratorToken;
    
    if(consume(ID)) {
        if(consume(LPAR)) { // optional
            while(expr()) {
                if(!consume(COMMA)) {
                    break;
                }
            }

            if(consume(RPAR)) { }
            else printTokenErrorAndExit("Missing ')'");
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
        iteratorToken = start;
    }

    iteratorToken = start;
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
            } else printTokenErrorAndExit("Missing '}'");
        }
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

    Token *start = iteratorToken;

    if(exprPrimary()) {
        if(exprPostfixPrim()) {
            return true;
        }
    }

    iteratorToken = start;
    return false;
}

bool exprUnary() {
#ifdef DEBUG
    printf("# exprUnary\n");
#endif

    Token *start = iteratorToken;

    if(consume(SUB) || consume(NOT)) {
        if(exprUnary()) {
            return true;
        }
        iteratorToken = start;
    }

    if(exprPostfix()) {
        return true;
    }

    iteratorToken = start;
    return false;
}

bool exprCast() {
#ifdef DEBUG
    printf("# exprCast\n");
#endif

    Token *start = iteratorToken;

    if(consume(LPAR)) {
        if(typeBase()) {
            if(arrayDecl()) { }

            if(consume(RPAR)) {
                if(exprCast()) {
                    return true;
                }
            }
            printTokenErrorAndExit("Missing ')'");
        }
        iteratorToken = start;
    }

    if(exprUnary()) {
        return true;
    }

    iteratorToken = start;
    return false;
}

bool exprMulPrim() {
#ifdef DEBUG
    printf("# exprMulPrim\n");
#endif

    if(consume(MUL) || consume(DIV)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        }
    }

    return true;
}

bool exprMul() {
#ifdef DEBUG
    printf("# exprMul\n");
#endif

    Token *start = iteratorToken;

    if(exprCast()) {
        if(exprMulPrim()) {
            return true;
        }
    }

    iteratorToken = start;
    return true;
}

bool exprAddPrim() {
#ifdef DEBUG
    printf("# exprAddPrim\n");
#endif

    if(consume(ADD) || consume(SUB)) {
        if(exprMul()) {
            if(exprAddPrim()) {
                return true;
            }
        }
    }

    return true;
}

bool exprAdd() {
#ifdef DEBUG
    printf("# exprAdd\n");
#endif

    Token *start = iteratorToken;

    if(exprMul()) {
        if(exprAddPrim()) {
            return true;
        }
    }

    iteratorToken = start;
    return false;
}

bool exprRelPrim() {
#ifdef DEBUG
    printf("# exprRelPrim\n");
#endif

    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }
    }

    return true;
}

bool exprRel() {
#ifdef DEBUG
    printf("# exprRel\n");
#endif

    Token *start = iteratorToken;

    if(exprAdd()) {
        if(exprRelPrim()) {
            return true;
        }
    }

    iteratorToken = start;
    return false;
}

bool exprEqPrim() {
#ifdef DEBUG
    printf("# exprEqPrim\n");
#endif

    if(consume(EQUAL) || consume(NOTEQ)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        }
    }

    return true;
}

bool exprEq() {
#ifdef DEBUG
    printf("# exprEq\n");
#endif

    Token *start = iteratorToken;

    if(exprRel()) {
        if(exprEqPrim()) {
            return true;
        }
    }

    iteratorToken = start;
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
    }

    return true;
}

bool exprAnd() {
#ifdef DEBUG
    printf("# exprAnd\n");
#endif

    Token *start = iteratorToken;

    if(exprEq()) {
        if(exprAndPrim()) {
            return true;
        }
    }

    iteratorToken = start;
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
    }

    return true;
}

bool exprOr() {
#ifdef DEBUG
    printf("# exprOr\n");
#endif

    Token *start = iteratorToken;

    // Recursivitate stanga
    if(exprAnd()) {
        if(exprOrPrim()) {
            return true;
        }
    }

    iteratorToken = start;
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
        }
        iteratorToken = start;
    }

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

    Token *start = iteratorToken;

    if(exprAssign()) {
        return true;
    }

    iteratorToken = start;
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