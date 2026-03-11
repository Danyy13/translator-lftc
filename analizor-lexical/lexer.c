#include <stdio.h>

#include "lexer.h"
#include "utils.h"

Token *tokenList = NULL;
Token *lastToken = NULL;

int line = 1;

Token *addToken(AtomCode code) {
    struct token *token = safeMalloc(sizeof(Token));

    token->code = code;
    token->line = line;
    // column

    token->next = NULL;
    // union

    if(lastToken) {
        lastToken->next = token;
    } else {
        tokenList = token;
    }
    lastToken = token;

    return token;
}

void showTokens(const Token *tokenList) {
    for(const Token *traverser=tokenList;traverser!=NULL;traverser=traverser->next) {
        printf("%d\n", traverser->code);
    }
}