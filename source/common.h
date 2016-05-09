#ifndef _COMMON_H_
#define _COMMON_H_

void atomicinit(void);
void atomiclock(void);
void atomicunlock(void);
unsigned int explode(char*, const char*, char***);
void array_free(char***, int);

#endif
