#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "utils.h"
#include "lexer.h"

extern Token *lastToken;

void printErrorAndExit(const char *format,  ...) {
    fprintf(stderr, "Error at line %d: ", lastToken->line);
    
    va_list va;
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

void *safeMalloc(size_t bytes) {
    void *ret = malloc(bytes);
    if(!ret) {
        printErrorAndExit("Malloc failed - Not enough memory");
        exit(EXIT_FAILURE);
    }
    return ret;
}

char *getFileContent(const char *pathname) {
    FILE *file = fopen(pathname, "rb");
    if(!file) printErrorAndExit("Unable to open %s", pathname);

    fseek(file, 0, SEEK_END); // Muta cursorul la finalul fisierului
    size_t fileSize = (size_t)ftell(file); // Numarul de BYTES pe care fisierul il contine
    fseek(file, 0, SEEK_SET); // Muta cursorul inapoi la inceputul fisierului

    char *buffer = (char *)safeMalloc((size_t)fileSize + 1);
    size_t itemsRead = fread(buffer, sizeof(char), (size_t)fileSize, file);
    
    // printf("Buffer: %s\n", buffer);

    if(fclose(file) != 0) printErrorAndExit("Error closing the file at %s", pathname);
    if(fileSize != itemsRead) printErrorAndExit("Could not read the whole file. Only read %d items from file %s", itemsRead, pathname);

    buffer[fileSize] = '\0'; // Punem identificatorul de final la sfarsitul fisierului
    
    return buffer;
}