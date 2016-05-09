// ini.h

#ifndef PLVIEW_INI_h
#define PLVIEW_INI_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define MAXLINE         256
#define MAXSTATIONS     10
#define MAXDATA         256
#define MAXFLDLEN       16

typedef struct{
    char* pr; 
    char* id;         
    char* ch;         
}Station;

void iniClean();
bool iniInit(char const*);
bool iniRead();
    
bool iniReadSection(char*);
bool iniReadValues(char*);   
bool iniAllocData();
void iniStrip(char*);
char* iniTrim(char*);

#endif
