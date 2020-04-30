#include "Utility.h"

/**
 * Gets the length of the string array
 * @param src	-> The array string
 * @return Returns the array length
 */
int str_len(char **src){
	int length = 0;

	if(src == NULL){
		return length;
	}

	while(src[length] != NULL){
		length++;
	}

	return length;
}

/**
 * Splits a string using a delimiter into an array
 * @param src	-> The string
 * @param delim	-> The delimiter
 * @return The splitted array
 */
char **str_split(char *src, const char *delim){
	char **result = malloc(1);
	char *str = strdup(src);

	char *token = strtok(str, delim);
	int index = 0;
	while(token != NULL){
		result = realloc(result, (index + 1) * sizeof(char *));
		result[index] = malloc(strlen(token) + 2);
		result[index] = strdup(token);

		index++;
		token = strtok(NULL, delim);
	}

	result = realloc(result, (index + 1) * sizeof(char *));
	result[index] = NULL;

	free(str);
	return result;
}

/**
 * Removes all occurencies of the character in the string
 * @param src	-> The string
 * @param delim	-> The character delimiter
 * @return Returns the string without the character
 */
char *str_removeChar(char *src, const char delim){
	char *str = strdup(src);
	char *result = (char *)malloc(strlen(str) + 2);

	int index = 0;
	for(int i = 0; i < strlen(str); i++){
		if(str[i] != delim){
			result[index++] = str[i];
		}
	}
	result[index] = '\0';


	free(str);
	return result;
}

/**
 * Frees the whole array
 * @param src	-> The string array
 */
void str_free(char **src){
	for(int i = 0; i < str_len(src); i++){
		free(src[i]);
	}
	free(src);
}
