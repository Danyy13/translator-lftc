#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

#define DEBUG_SHOW_TEXT_EXTRACTED 0

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

char *extractText(const char *start, const char *end) {
    int textLength = (int)(end - start);

    char *text = safeMalloc(textLength);
    
    strncpy(text, start, textLength);
    text[textLength] = '\0';

    return text;
}

AtomCode getKeywordAtomCode(const char *text) {
    if(strcmp(text, "int") == 0) {
        return TYPE_INT;
    } else if(strcmp(text, "char") == 0) {
        return TYPE_CHAR;
    } else if(strcmp(text, "double") == 0) {
        return TYPE_DOUBLE;
    } else if(strcmp(text, "else") == 0) {
        return ELSE;
    } else if(strcmp(text, "if") == 0) {
        return IF;
    } else if(strcmp(text, "return") == 0) {
        return RETURN;
    } else if(strcmp(text, "struct") == 0) {
        return STRUCT;
    } else if(strcmp(text, "void") == 0) {
        return VOID;
    } else if(strcmp(text, "while") == 0) {
        return WHILE;
    }

    return ID;
}

char getSpecialCharacter(char specialCharacterLetter) {
    switch(specialCharacterLetter) {
        case 'a': return '\a';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'v': return '\v';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0': return '\0';
        default: printErrorAndExit("The code should not reach this line. Please check special characters.");
    }
}

// Rule to follow: DOUBLE: [0-9]+ ( '.' [0-9]+ ( [eE] [+-]? [0-9]+ )? | ( '.' [0-9]+ )? [eE] [+-]? [0-9]+ )
void validateDoubleConstant(char *text) {
    // Pass all the first characters that are digits
    while(isdigit(*text)) {
        text++;
    }

    // Check '.' case
    if(*text == '.') {
        text++; // Pass the '.' char

        int digitsAfterDot = 0;
        while(isdigit(*text)) { // Pass all the digits 
            text++;
            digitsAfterDot++;
        }

        // If there are no digits after the dot, throw an error
        if(digitsAfterDot == 0) {
            printErrorAndExit("There must be one or more digits after the character '.'. Invalid character found: \'%c\'\n", *text);
        }
    }

    // Check 'e' or 'E' case
    if(*text == 'e' || *text == 'E') {
        text++; // Pass the 'e' char

        if(*text == '-' || *text == '+') {
            text++; // Pass char
        }

        int digitsAfterDot = 0;
        while(isdigit(*text)) { // Pass all the digits 
            digitsAfterDot++;
            text++;
        }

        if(digitsAfterDot == 0) {
            printErrorAndExit("There must be at least one or more digits right after the \"e(+|-)\" sequence");
        }
    }

    // printf("line: %d\tlast char: %c\n", line, *text);

    // If there is a non-null character after checking '.' and 'e' then throw error
    // Character belongs to ".eE+-" array but it should not be placed there for a good definition of the constant
    if(*text != '\0') {
        printErrorAndExit("Extra text after expected end of number. Invalid character \'%c\' after constant double declaration\n", *text);
    }
}

Token *tokenize(const char *pch) {
    showTokens(tokenList);
    
    for(;;) {
        switch(*pch) {
            // END
            case '\0':
                addToken(END);
                return tokenList;
            
            // Whitespaces
            case ' ': case '\t':
                pch++;
                break;
            case '\r': // handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
                if(pch[1] == '\n') pch++;
                // fallthrough to '\n'
            case '\n':
                line++; 
                pch++;
                break;

            // Delimiters
            case ',':
                addToken(COMMA);
                pch++;
                break;
            case ';':
                addToken(SEMICOLON);
                pch++;
                break;
            case '(':
                addToken(LPAR);
                pch++;
                break;
            case ')':
                addToken(RPAR);
                pch++;
                break;
            case '[':
                addToken(LBRACKET);
                pch++;
                break;
            case ']':
                addToken(RBRACKET);
                pch++;
                break;
            case '{':
                addToken(LACC);
                pch++;
                break;
            case '}':
                addToken(RACC);
                pch++;
                break;
            
            // Operators 
            case '+':
                addToken(ADD);
                pch++;
                break;
            case '-':
                addToken(SUB);
                pch++;
                break;
            case '*':
                addToken(MUL);
                pch++;
                break;
            case '/':
                if(pch[1] == '/') {
                    // Handle comment
                    while(pch[0] != '\n') pch++;
                    line++;
                    pch++;
                } else if(pch[1] == '*') {
                    // Handle multi-line comment
                    while(!(pch[0] == '*' && pch[1] == '/')) {
                        if(pch[0] == '\n') line++;
                        pch++;
                    }
                    pch += 2; // Skip last the */ characters
                } else {
                    addToken(DIV);
                    pch++;
                }
                break;
            case '.':
                addToken(DOT);
                pch++;
                break;
            case '&':
                if(pch[1] == '&') {
                    addToken(AND);
                    pch += 2;
                } else {
                    printErrorAndExit("Expected second '&' for AND atom");
                }
                break;
            case '|':
                if(pch[1] == '|') {
                    addToken(OR);
                    pch += 2;
                } else {
                    printErrorAndExit("Expected second '|' for OR atom");
                }
                break;
            case '=':
                if(pch[1] == '=') {
                    addToken(EQUAL);
                    pch += 2;
                } else {
                    addToken(ASSIGN);
                    pch++;
                }
                break;
            case '!':
                if(pch[1] == '=') {
                    addToken(NOTEQ);
                    pch += 2;
                } else {
                    addToken(NOT);
                    pch++;
                }
            case '<':
                if(pch[1] == '=') {
                    addToken(LESSEQ);
                    pch += 2;
                } else {
                    addToken(LESS);
                    pch++;
                }
            case '>':
                if(pch[1] == '=') {
                    addToken(GREATEREQ);
                    pch += 2;
                } else {
                    addToken(GREATER);
                    pch++;
                }

            // Default
            default:
                Token *token = NULL;

                // Int Constant
                if(isdigit(*pch)) {
                    int value = 0;
                    int isDouble = 0;
                    const char *start = pch;
                    const char doubleChars[] = ".eE-+";

                    while(isdigit(*pch) || (strchr(doubleChars, *pch) != NULL)) { // if the char is a digit or it is in doubleChars
                        if(strchr(doubleChars, *pch) != NULL) isDouble = 1;
                        pch++;
                    }

                    char *valueText = extractText(start, pch);

                    if(isDouble) {
                        validateDoubleConstant(valueText); // Check if the Double Constant is correct. In the case of an error, the function calls exit() from within
        
                        token = addToken(DOUBLE);
                        token->value.doubleValue = strtod(valueText, NULL);
                    } else {
                        token = addToken(INT);
                        token->value.intValue = strtol(valueText, NULL, 10);
                    }
                    
                    // printf("INT or DOUBLE text: %s\n", valueText);
                }

                // Char Constant
                if(*pch == '\'') {
                    const char *start = pch++;
                    char characterValue = 0;
                    
                    for(;*pch != '\'';pch++) {} // getting all characters in between ''
                    pch++;

                    if(start[1] == '\'' && start[2] == '\'') printErrorAndExit("Empty character constant");

                    const char specialCharacters[] = "abfnrtv\\\'\"0";
                    if(start[1] == '\\') {
                        if(start[3] != '\'') printErrorAndExit("Multi-character literal exceeds char type");

                        if(strchr(specialCharacters, start[2]) == NULL) printErrorAndExit("Only special characters can have \\ before them");
                        else characterValue = getSpecialCharacter(start[2]);
                    } else {
                        if(start[2] != '\'') printErrorAndExit("Multi-character literal exceeds char type");
                        characterValue = start[1];
                    }

                    token = addToken(CHAR);
                    token->value.charValue = characterValue;
                }
                
                // String Constant
                if(*pch == '\"') {
                    const char *start = ++pch;
                    int stringLength = 0;

                    for(;*pch != '\"';pch++) { // getting all characters in between ""
                        if(*pch == '\0') printErrorAndExit("Missing terminating \" character. String declaration started at line %d", line);
                    } 
                    pch++;
                    
                    // Get string value
                    stringLength = pch - start;
                    char *string = safeMalloc(stringLength * sizeof(char));
                    strncpy(string, start, stringLength - 1);
                    string[stringLength - 1] = '\0';

                    token = addToken(STRING);
                    token->value.text = string; // Punem structura sa pointeze la string-ul alocat dinamic
                }

                // Keywords and ID
                if(isalpha(*pch) || *pch == '_') {
                    const char *start = pch++;
                    for(;isalnum(*pch) || *pch == '_';pch++) {}
                    char *text = extractText(start, pch);
                    
                    if(DEBUG_SHOW_TEXT_EXTRACTED) printf("%s\n", text);
                                        
                    AtomCode keywordAtomCode = getKeywordAtomCode(text);

                    token = addToken(keywordAtomCode); // add code to struct
                    switch(keywordAtomCode) { // if necessary, add the value of the type to struct
                        case ID:
                            token->value.text = text;
                            break;
                        default:
                            break;
                    }
                }
                break;
        }
    }
}

const char* getAtomName(AtomCode atomCode) {
    switch(atomCode) {
        case ID:          return "ID";
        
        // keywords
        case TYPE_CHAR:   return "TYPE_CHAR";
        case TYPE_DOUBLE: return "TYPE_DOUBLE";
        case TYPE_INT:    return "TYPE_INT";
        case ELSE:        return "ELSE";
        case IF:          return "IF";
        case RETURN:      return "RETURN";
        case STRUCT:      return "STRUCT";
        case VOID:        return "VOID";
        case WHILE:       return "WHILE";
        
        // constants
        case INT:         return "INT";
        case DOUBLE:      return "DOUBLE";
        case CHAR:        return "CHAR";
        case STRING:      return "STRING";
        
        // delimiters
        case COMMA:       return "COMMA";
        case SEMICOLON:   return "SEMICOLON";
        case LPAR:        return "LPAR";
        case RPAR:        return "RPAR";
        case LBRACKET:    return "LBRACKET";
        case RBRACKET:    return "RBRACKET";
        case LACC:        return "LACC";
        case RACC:        return "RACC";
        case END:         return "END";
        
        // operators
        case ADD:         return "ADD";
        case SUB:         return "SUB";
        case MUL:         return "MUL";
        case DIV:         return "DIV";
        case DOT:         return "DOT";
        case AND:         return "AND";
        case OR:          return "OR";
        case NOT:         return "NOT";
        case ASSIGN:      return "ASSIGN";
        case EQUAL:       return "EQUAL";
        case NOTEQ:       return "NOTEQ";
        case LESS:        return "LESS";
        case LESSEQ:      return "LESSEQ";
        case GREATER:     return "GREATER";
        case GREATEREQ:   return "GREATEREQ";
        
        default:          return "UNKNOWN_ATOM";
    }
}

void printTokenValue(Token *token) {
    switch(token->code) {
        case CHAR:
            putchar(token->value.charValue);
            break;
        case INT:
            printf("%d", token->value.intValue);
            break;
        case DOUBLE:
            printf("%.2lf", token->value.doubleValue);
            break;
        case ID:
        case STRING:
            printf("%s", token->value.text);
            break;
        default:
            printErrorAndExit("Trying to print the value for a non-printable atom: %s", getAtomName(token->code));
            break;
    }
}

void printTokenValueToFile(FILE *outputFile, Token *token) {
    switch(token->code) {
        case CHAR:
            fputc(token->value.charValue, outputFile);
            break;
        case INT:
            fprintf(outputFile, "%d", token->value.intValue);
            break;
        case DOUBLE:
            fprintf(outputFile, "%.2lf", token->value.doubleValue);
            break;
        case ID:
        case STRING:
            fprintf(outputFile, "%s", token->value.text);
            break;
        default:
            printErrorAndExit("Trying to print the value for a non-printable atom: %s", getAtomName(token->code));
            break;
    }
}

void showTokens(Token *tokenList) {
    AtomCode constantsAndIdCode[] = {ID, INT, CHAR, STRING, DOUBLE, -1}; // These tokens also print the value

    for(Token *traverser=tokenList;traverser!=NULL;traverser=traverser->next) {
        int isConstantOrId = 0;
        for(int i=0;constantsAndIdCode[i]!=-1;i++) { // While code in array is not the last one, keep iterating
            if(constantsAndIdCode[i] == traverser->code) { // Check if the token has a constant or id code
                isConstantOrId = 1;
            }
        }
        
        if(isConstantOrId) { // For Id and Constants we also print the value
            printf("%d\t%s: ", traverser->line, getAtomName(traverser->code));
            printTokenValue(traverser);
            putchar('\n');
        } else {
            printf("%d\t%s\n", traverser->line, getAtomName(traverser->code));
        }
    }
}

void printTokensToFile(char *outputFilepath, Token *tokensList) {
    FILE *outputFile = fopen(outputFilepath, "w");
    if(!outputFile) {
        printErrorAndExit("Error opening file %s\n", outputFilepath);
    }

    AtomCode constantsAndIdCode[] = {ID, INT, CHAR, STRING, DOUBLE, -1}; // These tokens also print the value

    for(Token *traverser=tokenList;traverser!=NULL;traverser=traverser->next) {
        int isConstantOrId = 0;
        for(int i=0;constantsAndIdCode[i]!=-1;i++) { // While code in array is not the last one, keep iterating
            if(constantsAndIdCode[i] == traverser->code) { // Check if the token has a constant or id code
                isConstantOrId = 1;
            }
        }
        
        if(isConstantOrId) { // For Id and Constants we also print the value
            fprintf(outputFile, "%d\t%s: ", traverser->line, getAtomName(traverser->code));
            printTokenValueToFile(outputFile, traverser);
            fputc('\n', outputFile);
        } else {
            fprintf(outputFile, "%d\t%s\n", traverser->line, getAtomName(traverser->code));
        }
    }

    fclose(outputFile);
}
