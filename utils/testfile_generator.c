#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_FILEPATH "tests/testfile.c"

#define MAX_ID_SIZE 35
#define MAX_STRING_SIZE 50
#define MAX_BUFFER_SIZE 3000

char buffer[MAX_BUFFER_SIZE] = "";
unsigned int bufferIndex = 0;

void writeBufferToFile(char *filepath, char *buffer) {
    FILE *testFile = fopen(TEST_FILEPATH, "w");
    if(!testFile) {
        fprintf(stderr, "Eroare la deschidere fisier %s\n", TEST_FILEPATH);
        exit(EXIT_FAILURE);
    }

    fprintf(testFile, "%s\n", buffer);
    
    fclose(testFile);
}

int isBufferOverflow(int bufferIndex, int stringLength, int maxBufferSize) {
    return (bufferIndex + stringLength) > maxBufferSize; 
}

void checkBufferOverflow(int bufferIndex, int stringLength, int maxBufferSize) {
    if(isBufferOverflow(bufferIndex, stringLength, MAX_BUFFER_SIZE)) {
        fprintf(stderr, "Buffer Overflow Occurs. Printed last buffer state to file\n");
        writeBufferToFile(TEST_FILEPATH, buffer);
        exit(EXIT_FAILURE);
    }
}

void appendStringToBuffer(char *buffer, char *string) {
    int stringLength = strlen(string);
    checkBufferOverflow(bufferIndex, stringLength, MAX_BUFFER_SIZE);

    strncat(buffer, string, stringLength);
    bufferIndex += stringLength;
}

void writeIntAndIds() {
    const char ids[][MAX_ID_SIZE] = {"x", "Ok", "_ceva", "___", "sanki10", "1bet", "..bine", "$$$", ""};

    char lineString[MAX_STRING_SIZE];
    char intValueString[MAX_STRING_SIZE];

    for(int i=0;ids[i][0]!='\0';i++) {
        strcpy(lineString, "\tint ");
        strcat(lineString, ids[i]);
        strcat(lineString, " = ");
        strcat(lineString, itoa(i - 1, intValueString, 10));
        strcat(lineString, ";\n");

        appendStringToBuffer(buffer, lineString);
    }

    appendStringToBuffer(buffer, "\n");
}

void writeDoubles() {
    const char doubles[][MAX_ID_SIZE] = {"4.9", "3e5", "3.09e4", "300", "34e-1", "33.01e-2", "33.11ee3",  ""};

    char lineString[MAX_STRING_SIZE];
    char id[] = "a"; // id va creste alfabetic

    for(int i=0;doubles[i][0]!='\0';i++) {
        strcpy(lineString, "\tdouble ");
        strcat(lineString, id);
        id[0]++;
        strcat(lineString, " = ");
        strcat(lineString, doubles[i]);
        strcat(lineString, ";\n");

        appendStringToBuffer(buffer, lineString);
    }

    appendStringToBuffer(buffer, "\n");
}

int main() {
    // Write initial string to file
    char initialString[] = "int main(void) {\n";
    appendStringToBuffer(buffer, initialString);

    // Write Constant
    writeIntAndIds();
    writeDoubles();

    // Write final string to buffer
    char finalString[] = "\n\treturn 0;\n}";
    appendStringToBuffer(buffer, finalString);

    // Write to file
    writeBufferToFile(TEST_FILEPATH, buffer);

    return 0;
}