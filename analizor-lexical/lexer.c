#include <stdio.h>
#include <string.h>

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

char *extractText(char *start, char *end) {
    int textLength = (int)(end - start) + 1;

    char *text = safeMalloc(textLength);
    
    strncpy(text, start, textLength - 1);
    text[textLength] = '\0';

    return text;
}

Token *tokenize(const char *pch) {

    
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
                addToken(DIV);
                pch++;
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
            case '|':
                if(pch[1] == '|') {
                    addToken(OR);
                    pch += 2;
                } else {
                    printErrorAndExit("Expected second '|' for OR atom");
                }
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
            // default:
            //     if(isalpha(*pch) || *pch == '_') {
            //         for(const char *start=pch++; isalpha(*pch) || *pch == '_'; pch++) {}
            //         char *text = extractText(start, pch);
            //     }
        }
    }
}

void showTokens(const Token *tokenList) {
    for(const Token *traverser=tokenList;traverser!=NULL;traverser=traverser->next) {
        printf("%d\n", traverser->code);
    }
}