#include <stdio.h>

#include "analizor-lexical/lexer.h"

int main() {
    Token *tokenList = NULL;
    Token *lastToken = NULL;

    tokenList = addToken(ID);
    addToken(TYPE_DOUBLE);

    showTokens(tokenList);

    return 0;
}