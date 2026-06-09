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

void masinaVirtuala() {
    // Instruction *testCode = genTestProgram();
    // run(testCode);

    Instruction *testCodeDouble = genTestProgramDouble();
    if(testCodeDouble == NULL) {
        printErrorAndExit("Error with generation of the lab test program code (with double)");
    }
    run(testCodeDouble);
}

void generareCod() {
    Symbol *symbolMain = findSymbolInDomain(symbolTable, "main");
    if(!symbolMain) printErrorAndExit("Missing main function");

    Instruction *entryCode = NULL;
    addInstruction(&entryCode, OP_CALL)->arg.instruction = symbolMain->function.instruction;
    addInstruction(&entryCode, OP_HALT);
    run(entryCode);
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

    // masinaVirtuala();
    generareCod();

    // printf("OK dupa\n");

    // showDomain(symbolTable, "global"); // domain    
    dropDomain(); // domain

    free(tokenList);

    return 0;
}