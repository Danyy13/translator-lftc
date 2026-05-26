#include <stdio.h>
#include <stdlib.h>

#include "analizor-lexical/lexer.h"
#include "analizor-lexical/utils.h"
#include "analizor-sintactic/parser.h"
#include "analizor-domeniu/domain.h"
#include "masina-virtuala/vm.h"

#define LEXER_OUTPUT_FILEPATH "lexer_out.txt"

Token *analizorLexical(char *testFilePath) {
    Token *tokenList = NULL;

    char *fileContent = getFileContent(testFilePath);

    tokenList = tokenize(fileContent);

    printTokensToFile(LEXER_OUTPUT_FILEPATH, tokenList);

    return tokenList;
}

void analizorSintactic(Token *tokenList) {
    parse(tokenList);
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Invalid number of arguments. Usage: ./app.exe {inputFilePath}");
        exit(EXIT_FAILURE);
    }

    char *testFilePath = argv[1];

    Token *tokenList = analizorLexical(testFilePath);

    pushDomain(); // domain
    vmInit();
    analizorSintactic(tokenList);

    // Instruction *testCode = genTestProgram();
    // run(testCode);

    Instruction *testCodeDouble = genTestProgramDouble();
    run(testCodeDouble);

    // showDomain(symbolTable, "global"); // domain    
    dropDomain(); // domain

    free(tokenList);

    return 0;
}