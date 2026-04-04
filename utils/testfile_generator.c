#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_FILEPATH "tests/testfile.c"

// Comment the cases you don't want to generate in the test file
#define DOUBLE_ERRORS
#define CHAR_ERRORS
#define STRING_ERRORS

// Max sizes for different types of strings
#define MAX_ID_SIZE 35
#define MAX_STRING_SIZE 50
#define MAX_LINE_SIZE 200
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

int isValidId(char *id) {
    if(id[0] < 'a' || id[0] > 'z') {
        return 0;
    }

    return 1;
}

void appendStringToBuffer(char *buffer, char *string) {
    int stringLength = strlen(string);
    checkBufferOverflow(bufferIndex, stringLength, MAX_BUFFER_SIZE);

    strncat(buffer, string, stringLength);
    bufferIndex += stringLength;
}

void writeIntAndIds() {
    char ids[][MAX_ID_SIZE] = {"x", "Ok", "_ceva", "___", "sanki10", // Not errors
        // "1bet", "..bine", "$$$", // Errors
        "" // End character - MANDATORY
    };

    char lineString[MAX_LINE_SIZE];
    char intValueString[MAX_STRING_SIZE];

    for(int i=0;ids[i][0]!='\0';i++) {
        char *variableName = ids[i];
        char *variableValue = itoa(i - 1, intValueString, 10);

        sprintf(lineString, "\tint %s = %s;\n", variableName, variableValue);

        appendStringToBuffer(buffer, lineString);
    }

    appendStringToBuffer(buffer, "\n");
}

void writeDoubles() {
    char doubles[][MAX_ID_SIZE] = {
        // Valid Edge Cases
        "0.0", "4.9", "300", "123e+4", "123e4", "34e-1", "0.000000001",      
#ifdef DOUBLE_ERRORS
        // Invalid / Tricky Cases
        "1e3.432"               // Dot after e
        ".5e3",                 // No integer part before dot
        "33.11ee3",             // Double exponent character
        "1.",                   // Missing trailing decimals
        "1.2.3",                // Multiple decimal points
        "1e",                   // Incomplete exponent
        "1.5e+",                // Incomplete exponent with sign
#endif
        "" // End character - MANDATORY
    };

    char lineString[MAX_LINE_SIZE];
    char id[] = "a"; // id va creste alfabetic

    for(int i=0;doubles[i][0]!='\0';i++) {
        char *variableValue = doubles[i];

        sprintf(lineString, "\tdouble %s = %s;\n", id, variableValue);

        if(!isValidId(id)) {
            fprintf(stderr, "Reached maximum distinct id names (a-z). Stopped generating at %s\n", lineString);
            break;
        }
        id[0]++;

        appendStringToBuffer(buffer, lineString);
    }

    appendStringToBuffer(buffer, "\n");
}

void writeStrings() {
    char strings[][MAX_STRING_SIZE] = {
        // Valid Edge Cases
        "!@#$%^&*()_+",
        "ana are mere\\n\\n\\n", 
        "This contains a \\\"quote\\\" inside", // Escaped double quote
        "Path\\\\to\\\\file",                   // Escaped backslashes
        " ",                                    // Single space string
    #ifdef STRING_ERRORS
        // Invalid / Tricky Cases
        "trebuia sa zic \"Ce mai faci?\" dar nu am apucat",
        "String with an actual\\nnewline",      // Should trigger an error if unescaped newlines aren't allowed
    #endif
        "" // End character - MANDATORY
    };

    char lineString[MAX_LINE_SIZE];
    char id[] = "a";

    for(int i=0;strings[i][0] != '\0';i++) {
        char *variableValue = strings[i];

        sprintf(lineString, "\tchar %s[] = \"%s\";\n", id, variableValue);

        if(!isValidId(id)) {
            fprintf(stderr, "Reached maximum distinct id names (a-z). Stopped generating at %s\n", lineString);
            break;
        }
        id[0]++;

        appendStringToBuffer(buffer, lineString);
    }

    appendStringToBuffer(buffer, "\n");
}

void writeChars() {
    char chars[][MAX_STRING_SIZE] = {
        // Valid Edge Cases
        "A", " ", "\\0", "\\'", "\\\\", "\\n", "\\t",
#ifdef CHAR_ERRORS
        // Invalid / Tricky Cases
        "\\\\\\",               // Unescaped trailing slash
        "ab",                   // Multiple characters
        "\\x",                  // Invalid escape sequence
#endif
        "" // End character - MANDATORY
    };

    char lineString[MAX_LINE_SIZE];
    char id[] = "a";

    for(int i=0;chars[i][0] != '\0';i++) {
        char *variableValue = chars[i];

        sprintf(lineString, "\tchar %s = '%s';\n", id, variableValue);

        if(!isValidId(id)) {
            fprintf(stderr, "Reached maximum distinct id names (a-z). Stopped generating at %s\n", lineString);
            break;
        }
        id[0]++;

        appendStringToBuffer(buffer, lineString);
    }

    appendStringToBuffer(buffer, "\n");
}

void writeIfElse() {
    char lineString[MAX_LINE_SIZE];
    
    char condition[] = "3.05 == 305.000e-2";
    char ifExpression[] = "puts(\"ok\");";
    char elseExpression[] = "putchar('a');";

    sprintf(lineString, "\tif(%s) {\n\t\t%s\n\t} else {\n\t\t%s\n\t}\n", condition, ifExpression, elseExpression);

    appendStringToBuffer(buffer, lineString);
}

int main() {
    // Write initial string to file
    char initialString[] = "int main(void) {\n";
    appendStringToBuffer(buffer, initialString);

    // Write Constant
    writeIntAndIds();
    writeDoubles();
    writeChars();
    writeStrings();

    writeIfElse();

    // Manual Edge Cases (Bypassing array limitations)
    appendStringToBuffer(buffer, "\n\t// Manual Edge Cases\n");
    
    // Empty String & Empty Char
    appendStringToBuffer(buffer, "\tchar empty_str[] = \"\";\n");
    appendStringToBuffer(buffer, "\tchar empty_char = '';\n"); // Usually an error in C

    // Unclosed String & Unclosed Char (Lexer should error and not crash)
    appendStringToBuffer(buffer, "\tchar unclosed_str[] = \"This string never ends;\n");
    appendStringToBuffer(buffer, "\tchar unclosed_char = 'a;\n");

    // Write final string to buffer
    char finalString[] = "\n\treturn 0;\n}";
    appendStringToBuffer(buffer, finalString);

    // Write to file
    writeBufferToFile(TEST_FILEPATH, buffer);

    return 0;
}