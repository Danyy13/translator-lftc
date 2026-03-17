#include <stdio.h>

#include "analizor-lexical/lexer.h"
#include "analizor-lexical/utils.h"

void analizorLexical() {
    Token *tokenList = NULL;
    Token *lastToken = NULL;

    char *fileContent = getFileContent("testfile.c");

    tokenList = tokenize(fileContent);

    showTokens(tokenList);
}

int main() {
    analizorLexical();
    return 0;
}