#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"

Token *iteratorToken; // the iterator in the tokens list
Token *consumedToken; // the last consumed token

bool typeBase() {
    if(consume(TYPE_INT)) return true;
    if(consume(TYPE_DOUBLE)) return true;
    if(consume(TYPE_CHAR)) return true;
    
    Token *start = iteratorToken; // only necessary for struct declaration

    if(consume(STRUCT)) {
        if(consume(ID)) {
            return true;
        }
    }

    iteratorToken = start;
    return false;
}

bool arrayDecl() {
    Token *start = iteratorToken;

    if(consume(LBRACKET)) {
        if(consume(INT)) { } // optional INT

        if(consume(RBRACKET)) {
            return true;
        }
    }

    iteratorToken = start;
    return false;
}

bool varDef() {
    Token *start = iteratorToken;

    if(typeBase()) {
        if(consume(ID)) {
            if(arrayDecl()) { }

            if(consume(SEMICOLON)) {
                return true;
            }

            // no semicolon error
        }
        // no ID error
    }

    iteratorToken = start;
    return false;
}

bool structDef() {
    Token *start = iteratorToken;

    if(consume(STRUCT)) {
        if(consume(ID)) {
            
        }
    }

    iteratorToken = start;
    return false;
}

void printTokenErrorAndExit(const char *fmt, ...) {
    frprintf(stderr, "Error in line %d: ", iteratorToken->line);
    
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

bool consume(int code) {
    if(iteratorToken->code == code) {
        consumedToken = iteratorToken;
        iteratorToken = iteratorToken->next;

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