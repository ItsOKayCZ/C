#ifndef UTILITY_H
#define UTILITY_H

// Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Public functions
int str_len(char **src); 
char **str_split(char *src, const char *delim);
char *str_removeChar(char *src, const char delim);
void str_free(char **str); 

#endif
