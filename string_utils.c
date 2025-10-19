#include "string_utils.h"
#include <string.h>
#include <stdlib.h>

// check pattern found in string
int match_found(int start, char *str, char *pattern){
    int patternLen = strlen(pattern);
    
    // compare current group with pattern
    for(int i = start, k = 0; i < patternLen + start; i++, k++){
        if (str[i] != pattern[k]) return 0;
    }
    return 1;
}

// replace all match strings in string into another one
char *stringutils_replace(const char *original,const char *pattern,const char *newSubstr, char* out){

    char *newStr = out;
    int i;

    int newIndex = 0;
    int oldLength = strlen(original);
    int newLength = strlen(newSubstr);
    int patternLen = strlen(pattern);

    int end = oldLength - oldLength % patternLen;

    for(i = 0; i < end; ){
        
        if (match_found(i, original, pattern)){
            for(int k = 0; k<newLength; k++, newIndex++){
                newStr[newIndex] = newSubstr[k];
            }
            i += patternLen;
            continue;
        }

        newStr[newIndex] = original[i];
        newIndex++;
        i++;
    }

    for(; i<oldLength; i++, newIndex++){
        newStr[newIndex] = original[i];
    }

    newStr[newIndex] = '\0';
    return newStr;
}

// get count of character in string
size_t stringutils_strcnt(const char* s, char c){
	size_t cnt = 0;
	char* t = strchr(s, c);
	while(t != NULL){
		cnt++;
        t = strchr(t + 1, c);
	}	

	return cnt;
}

// eat up until string matched with input
char* stringutils_consume_or_null(const char* str, const char* input){
	size_t len = strlen(input);
	size_t s_len = strlen(str);
	if(s_len < len)
		return NULL;
	char *p = str;
	for(int i = 0; i < len; i++){
		if(*p != str[i]){
			return NULL;
		}

		p++;
	}

	return p;
}

// find string that fulfills predicate function in string array
char* stringutils_find_any(const char** clist, size_t c, const char* str, int (* predicate)(const char*, const char*)){
	for(int i = 0; i < c; i++){
        
		if(clist[i][0] != '\0' && predicate(clist[i], str))
			return clist[i];
	}

	return NULL;
}

// check string startwith another string
int stringutils_strstartwith(const char* s, const char* t){
	return strncmp(s, t, strlen(t)) == 0;
}

// check string string equals
int stringutils_strequal(const char* s, const char* t){
	return strcmp(s,t) == 0;
}

// check string startwith another string
int stringutils_startwith(const char* s, const char* t){
	return stringutils_strstartwith(s,t);
}
