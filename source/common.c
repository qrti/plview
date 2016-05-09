#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <pthread.h>

#include "mem.h"

static pthread_mutex_t atomic_lock;
static pthread_mutexattr_t atomic_attr;

void atomicinit(void)
{
	pthread_mutexattr_init(&atomic_attr);
	pthread_mutexattr_settype(&atomic_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&atomic_lock, &atomic_attr);
}

void atomiclock(void) 
{
	pthread_mutex_lock(&atomic_lock);
}

void atomicunlock(void)
{
	pthread_mutex_unlock(&atomic_lock);
}

unsigned int explode(char *str, const char *delimiter, char ***output) 
{
	if(str==NULL || output==NULL) 
		return 0;
	
	unsigned int i = 0, n = 0, y = 0;
	size_t l = 0, p = 0;
	
	if(delimiter != NULL){
		l = strlen(str);
		p = strlen(delimiter);
	}
	
	while(i < l){
		if(strncmp(&str[i], delimiter, p) == 0){
			if(i - y > 0){
				*output = REALLOC(*output, sizeof(char *)*(n + 1));
				(*output)[n] = MALLOC(i - y + 1);
				strncpy((*output)[n], &str[y], i - y);
				(*output)[n][(i - y)] = '\0';
				n++;
			}
			
			y = i + p;
		}
		
		i++;
	}
	
	if(strlen(&str[y]) > 0){
		*output = REALLOC(*output, sizeof(char *)*(n + 1));
		(*output)[n] = MALLOC(i - y + 1);
		strncpy((*output)[n], &str[y], i - y);
		(*output)[n][(i - y)] = '\0';
		n++;
	}
	
	return n;
}

void array_free(char ***array, int len) 
{
	int i = 0;
	
	if(len > 0){
		for(i=0; i<len; i++)
			FREE((*array)[i]);
		
		FREE((*array));
	}
}
