#pragma once

// TODO: add all atoms from file into enum
typedef enum atomCode {
	ID,
	// keywords
	TYPE_CHAR, TYPE_DOUBLE, TYPE_INT, ELSE, IF, RETURN, STRUCT, VOID, WHILE,
	// constants
	INT, DOUBLE, CHAR, STRING,
	// delimiters
	COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC, END,
	// operators
	ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ
}AtomCode;

// TODO: add column (OPTIONAL!)
typedef struct token{
	AtomCode code;		// ID, TYPE_CHAR, ...
	int line;		// the line from the input file
    int column;     // the column from the input file
	
    union Data {
		char *text;		// the chars for ID, STRING (dynamically allocated)
		int i;		// the value for INT
		char c;		// the value for CHAR
		double d;		// the value for DOUBLE
	}value;

	struct token *next;		// next token in a simple linked list
}Token;

typedef struct tokenList {
    Token *head;
    Token *tail;
}TokenList;

Token *addToken(AtomCode code);
Token *tokenize(const char *pch);
void showTokens(Token *tokens);