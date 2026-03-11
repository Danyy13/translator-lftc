#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "utils.h"

void printError(const char *format,  ...) {
    fprintf(stderr, "Error: ");
    
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
        printError("Not enough memory");
        exit(EXIT_FAILURE);
    }
    return ret;
}

char *getFileContent(const char *pathname) {
    FILE *file = fopen(pathname, "rb");
    if(!file) printError("Unable to open %s", pathname);

    fseek(file, 0, SEEK_END); // Moves file cursor to the end of file
    size_t fileSize = (size_t)ftell(file); // Numarul de BYTES pe care fisierul in contine

    char *buffer = (char *)safeMalloc((size_t)fileSize + 1);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    
    if(fclose(file) != 0) printError("Error closing the file at %s", pathname);
    if(fileSize != bytesRead) printError("Could not read the whole file. Only read %d bytes from file %s", bytesRead, pathname);

    buffer[fileSize] = '\0'; // Punem identificatorul de final la sfarsitul fisierului
    
    return buffer;
}