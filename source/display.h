#ifndef _DISPLAY_H_
#define _DISPLAY_H_

// attributes <ESC>[{attr1};...;{attrn}m
#define ARES    "\033[0m"
#define ABRI    "\033[1m"
#define ADIM    "\033[2m"
#define AUND    "\033[4m"
#define ABLI    "\033[5m"
#define AREV    "\033[7m"
#define AHID    "\033[8m"

// foreground colors
#define FBLA    "\033[30m"
#define FRED    "\033[31m"
#define FGRN    "\033[32m"
#define FYEL    "\033[33m"
#define FBLU    "\033[34m"
#define FMAG    "\033[35m"
#define FCYN    "\033[36m"
#define FWHT    "\033[37m"

// background colors
#define BBLA    "\033[40m"
#define BRED    "\033[41m"
#define BGRN    "\033[42m"
#define BYEL    "\033[43m"
#define BBLU    "\033[44m"
#define BMAG    "\033[45m"
#define BCYN    "\033[46m"
#define BWHT    "\033[47m"

// \u25c0 left \u25b6 right unknown to putty
#define ARROW_UP       "\u25b2"  
#define ARROW_DOWN     "\u25bc"  
#define ARROW_LEFT     "\u25c4"
#define ARROW_RIGHT    "\u25ba"

// https://en.wikipedia.org/wiki/ANSI_escape_code
#define DOT         "\033[44;34m \033[0m"
#define TRENDC      "\033[33m"    

#define REC_OK      "\033[42;30m" 
#define REC_ER      "\033[41;33m"

// display width
#define DISPWIDTH   60
// number of stations
#define STATIONS    3

void display(int, double, double);
void updateTempHumi(int, double, double);
void updateTrend();
void updateRx();
void printTempHumi();
void printRx();
void printTrend();
void printTr(int, int,int);
int trend_temp(int, int);
int trend_humi(int, int);

#endif

//// https://en.wikipedia.org/wiki/ANSI_escape_code
//#define DOT         "\033[44;34m \033[0m"
//#define TRENDC      "\033[33m"    
//
//#define COL256
//
//#ifdef COL8
//#define REC_OK      "\033[44m" 
//#define REC_ER      "\033[41m"
//#endif
//
//// ESC[38;5;#m  foreground
//// ESC[48;5;#m  background
//#ifdef COL256
//#define REC_OK      "\033[48;5;28;33m"
//#define REC_ER      "\033[48;5;196;33m"
//#endif
//
//// ESC[38;2;r;g;bm   foreground
//// ESC[48;2;r;g;bm   background
//#ifdef COL16M
//#define REC_OK      "\033[48;2;0;135;0;33m"
//#define REC_ER      "\033[48;2;255;0;0;33m"
//#endif
