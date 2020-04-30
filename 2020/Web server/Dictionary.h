#ifndef DICTIONARY_H
#define DICTIONARY_H

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Including headers
#include "Utility.h"

/**
 * The main dictionary structure
 * key		-> The key
 * value	-> The value
 */
typedef struct{
	char *key;
	char *value;
} Dictionary;

// Functions
void dictionary_add(Dictionary ***dict, char *key, char *value);
char *dictionary_findValue(Dictionary **dict, char *key);
void dictionary_free(Dictionary **dict);
Dictionary **dictionary_init(char **src, const char *delim);

#endif
