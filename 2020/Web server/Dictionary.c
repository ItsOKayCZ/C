#include "Dictionary.h"

/**
 * Add an element into the dictionary
 * @param dict	-> The dictionary
 * @param key	-> The key to add into the dictionary
 * @param value	-> The value to assign to the key
 */
void dictionary_add(Dictionary ***dict, char *key, char *value){
	Dictionary *temp = (Dictionary *)malloc(sizeof(Dictionary));
	temp->key = strdup(key);
	temp->value = strdup(value);

	int size = dictionary_size(*dict);
	*dict = realloc(*dict, (size + 2) * sizeof(Dictionary **));
	(*dict)[size] = temp;
	(*dict)[size + 1] = NULL;
}

/**
 * Find the value of the key in the dictionary
 * @param dict	-> The dictionary
 * @param key	-> The key
 * @return Returns the value from the dictionary
 */
char *dictionary_findValue(Dictionary **dict, char *key){
	for(int i = 0; i < dictionary_size(dict); i++){
		if(strcmp(dict[i]->key, key) == 0){
			return dict[i]->value;
		}
	}

	return NULL; 
}

/**
 * Frees the memory of the dictionary 
 * @param dict -> The dictionary
 */
void dictionary_free(Dictionary **dict){
	for(int i = 0; i < dictionary_size(dict); i++){
		free(dict[i]->key);
		free(dict[i]->value);
	}
	free(dict);
}

/**
 * Gets the size of the dictionary 
 * @param dict	-> The dictionary
 */
int dictionary_size(Dictionary **dict){
	int size = 0;

	if(dict == NULL){
		return size;
	}

	while(dict[size] != NULL){
		size++;
	}

	return size;
}

/**
 * Makes a dictionary from a string
 * @param src	-> The string that will be formated to the dictionary
 * @param delim	-> The delimiter that will split the string into the keys and values
 * @return Returns the dictionary structure with the keys and values set
 */
Dictionary **dictionary_init(char **src, const char *delim){
	Dictionary **dict = malloc(1);

	int index = 0;
	for(int i = 0; i < str_len(src); i++){

		char **attributes = str_split(src[i], delim);

		for(int j = 2; j < str_len(attributes); j++){
			attributes[1] = (char *)realloc(attributes[1], strlen(attributes[1]) + strlen(attributes[j]) + 3);
			strcat(attributes[1], ":");
			strcat(attributes[1], attributes[j]);
			free(attributes[j]);
		}

		if(attributes[0] == NULL || attributes[1] == NULL || strcmp(attributes[0], "") == 0 || strcmp(attributes[1], "") == 0){
			break;
		}

		if(attributes[1][0] == ' '){
			attributes[1]++;
		}

		Dictionary *element = (Dictionary *)malloc(sizeof(Dictionary));
		element->key = strdup(attributes[0]);
		element->value = strdup(attributes[1]);

		dict = (Dictionary **)realloc(dict, (i + 1) * sizeof(Dictionary *));
		dict[i] = element;
		
		index = i + 1;
	}

	dict = (Dictionary **)realloc(dict, (index + 1) * sizeof(Dictionary *));
	dict[index] = NULL;

	return dict;
}
