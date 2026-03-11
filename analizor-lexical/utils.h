// requires at least C11
// in Visual Studio it is set from Properties -> C/C++ -> C Language Standard 
#pragma once // Header Guard - Directiva de preprocesor care previne includerea multipla a fisierelor

#include <stddef.h>
#include <stdnoreturn.h>

// prints to stderr a message prefixed with "error: " and exit the program
// the arguments are the same as for printf
noreturn void err(const char *format, ...); // noreturn - spune ca functia nu returneaza ceva sau nu ajunge la finalul corpului functiei

// allocs memory using malloc
// if succeeds, it returns the allocated memory, else it prints an error message and exit the program
void *safeMalloc(size_t bytes);

// loads a text file in a dynamically allocated memory and returns its content
// on error, prints a message and exit the program
char *getFileContent(const char *pathname);