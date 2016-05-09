#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>

#include "display.h"
#include "mem.h"

#define FIRSTCHAR   ' '
#define CHARHEIGHT  6

char widthtab[] = {
    0x05,0x00,0x00,0x00,0x00,0x05,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x03,0x01,0x00,
    0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x03 
};

char chartab[] = {
    0x00,0x00,0x00,0x00,0x00,   // SPACE
    0x22,0x10,0x08,0x04,0x22,   // %
    0x04,0x0a,0x04,             // '        
    0x00,0x00,0x00,             // +        
    0x04,0x04,0x04,             // -        
    0x20,                       // .                  
    0x1e,0x29,0x25,0x23,0x1e,   // 0
    0x00,0x04,0x02,0x3f,0x00,   // 1
    0x22,0x31,0x29,0x25,0x22,   // 2
    0x12,0x21,0x25,0x25,0x1a,   // 3
    0x03,0x04,0x04,0x04,0x3e,   // 4
    0x27,0x25,0x25,0x25,0x19,   // 5
    0x1e,0x25,0x25,0x25,0x18,   // 6
    0x00,0x01,0x39,0x05,0x03,   // 7
    0x1a,0x25,0x25,0x25,0x1a,   // 8
    0x02,0x25,0x25,0x25,0x1e,   // 9
    0x1c,0x22,0x22              // C
};

double cuva[STATIONS][2];               // current values - station, temperature/humidity

char rxhist[STATIONS][DISPWIDTH];       // receive history
char rx[STATIONS];                      // receive
char init[STATIONS];                    // init

// avs = avs + current - avg; avg = avs / avc;     
// average speed avc, fast and slow in minutes
#define FAST     5  
#define SLOW    60

long avs[STATIONS][2][2];               // average sum - station, slow/fast, temperature/humidity
long avg[STATIONS][2][2];               // average 

int conwidth;                           // console width
int gap;

void display(int station, double temp, double humi)
{    
    static bool first = true;
    
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    conwidth = ws.ws_col>DISPWIDTH ? DISPWIDTH : ws.ws_col;     
    gap = ws.ws_row<=22 ? 1 : 2;
           
    if(first){
        printf("\033[2J");
        printTempHumi();
        printTrend();
        printRx();
        first = false;            
    }
    else if(station != -1){
        updateTempHumi(station, temp, humi);   
    }
    else if(station == -1){
        updateTrend();
        updateRx();
        
        printTempHumi();
        printTrend();
        printRx();
    }
}

void updateTempHumi(int station, double temp, double humi)
{
    cuva[station][0] = temp;
    cuva[station][1] = humi;
    
    if(init[station] == 0)
        init[station] = 1;       
        
    rx[station] = 1;    
}

#define LEFTTEMP   33
#define LEFTHUMI   57

void updateTrend()
{    
    for(int s=0; s<STATIONS; s++){
        int temp = round(cuva[s][0] * 10);
        int humi = cuva[s][1];     
            
        if(init[s] == 1){
            avs[s][0][0] = temp * FAST;
            avs[s][0][1] = humi * FAST;
            avs[s][1][0] = temp * SLOW;
            avs[s][1][1] = humi * SLOW;
            
            avg[s][0][0] = temp;
            avg[s][0][1] = humi;
            avg[s][1][0] = temp;
            avg[s][1][1] = humi;
            
            init[s] = 2;

            return;
        }            
        else if(init[s] == 2){
            avs[s][0][0] += temp - avg[s][0][0]; avg[s][0][0] = avs[s][0][0] / FAST;
            avs[s][0][1] += humi - avg[s][0][1]; avg[s][0][1] = avs[s][0][1] / FAST;
            avs[s][1][0] += temp - avg[s][1][0]; avg[s][1][0] = avs[s][1][0] / SLOW;
            avs[s][1][1] += humi - avg[s][1][1]; avg[s][1][1] = avs[s][1][1] / SLOW;
        }
    }
}

void updateRx()
{   
    for(int s=0; s<STATIONS; s++){
        for(int x=0; x<conwidth-1; x++)
            rxhist[s][x] = rxhist[s][x+1]; 
    
        rxhist[s][conwidth-1] = rx[s];
        rx[s] = 0;
    }
}

void printTempHumi()
{
    for(int s=0; s<STATIONS; s++){
        double temp = cuva[s][0];
        double humi = cuva[s][1];
        
        char buf[DISPWIDTH];
        sprintf(buf, "%c%4.1f'C %2.0f%%", temp<0 ? '-' : '+', temp<0 ? -temp : temp, humi);    
            
        for(int y=0; y<CHARHEIGHT; y++){    
            printf("\033[%d;1H", s*(CHARHEIGHT+gap)+y+1);  
            int col = 0;
              
            for(int c=0; c<strlen(buf); c++){
                int p = 0;
                int n = buf[c] - FIRSTCHAR;
                
                for(int i=0; i<n; i++)
                    p += widthtab[i];

                for(int x=0; x<widthtab[n] && col<conwidth; x++, col++)
                    printf("%s", chartab[p + x] & (1 << y) ? DOT : " ");  
                        
                if(c>0 && col++<conwidth)
                    printf(" ");
            }
            
            while(col++ < conwidth)
                printf(" ");
                
            fflush(stdout);    
        }            
    }
}

void printTrend()
{
    printf(TRENDC);
    
    for(int s=0; s<STATIONS; s++){
        int ttemp = trend_temp(avg[s][0][0], avg[s][1][0]);     
        int thumi = trend_humi(avg[s][0][1], avg[s][1][1]);      
        
        printTr(ttemp, s*(CHARHEIGHT+gap)+1, LEFTTEMP);
        printTr(thumi, s*(CHARHEIGHT+gap)+1, LEFTHUMI);
        
        printf("\033[%d;1H", s*(CHARHEIGHT+gap)+CHARHEIGHT+1);
    }
    
    printf(ARES);
    fflush(stdout);         
}

void printTr(int trend, int top, int left)
{
    if(trend == 0){
        printf("\033[%d;%dH", top, left);
        printf(ARROW_RIGHT);
    }
    else{
        for(int t=0; t<abs(trend); t++){
            printf("\033[%d;%dH", top+t, left);
            printf("%s", trend<0 ? ARROW_DOWN : ARROW_UP);
        }
    }    
}

void printRx()
{
    for(int s=0; s<STATIONS; s++){
        int x = 0;
        
        for(x=0; x<conwidth; x++)
            if(rxhist[s][x])
                break;

        printf("\033[%d;1H", s*(CHARHEIGHT+gap)+CHARHEIGHT+1);
        printf("%s", x<conwidth ? REC_OK : REC_ER);
        
        for(x=0; x<conwidth; x++)
            printf("%c", rxhist[s][x] ? '*' : ' ');
    }
    
    printf(ARES);   
    fflush(stdout);  
}

int trend_temp(int fast, int slow)
{
    static int step[] = { 5, 10, 20, 30, 40, 50 };      // 1/10 degrees
    
    int dt = fast - slow;
    int t;
    
    for(t=0; t<CHARHEIGHT; t++)
        if(abs(dt) < step[t])
            break;
        
    return dt<0 ? -t : t;     
}

int trend_humi(int fast, int slow)
{
    static int step[] = { 2, 4, 8, 10, 12, 14 };        // %
    
    int dt = fast - slow;
    int t;
    
    for(t=0; t<CHARHEIGHT; t++)
        if(abs(dt) < step[t])
            break;
        
    return dt<0 ? -t : t;     
}

// time_t rawtime = time(NULL);
// struct tm *ti = localtime(&rawtime);
// sprintf(buf, "%40s%02d.%02d.%04d %02d:%02d:%02d ", " ", ti->tm_mday, ti->tm_mon + 1, ti->tm_year + 1900, ti->tm_hour, ti->tm_min, ti->tm_sec);

//void printTrend(int trend, int top, int left)
//{
//    if(trend == 0){
//        return;
//    }
//    else if(trend > 0){
//        for(int t=1; t<=trend; t++){
//            printf("\033[%d;%dH", top+t, left+3-t);
//            
//            for(int x=0; x<(t-1)*2+1; x++)
//                printf("%s", ARROW_UP);
//        }
//    }
//    else{
//        for(int t=-1; t>=trend; t--){
//            printf("\033[%d;%dH", top+CHARHEIGHT+t+1, left+3+t);
//            
//            for(int x=0; x<(-t-1)*2+1; x++)
//                printf("%s", ARROW_DOWN);
//        }
//    }    
//}

//int trend_log2(int fast, int slow, int fac)
//{
//    int dt = fast - slow;
//    
//    if(dt == 0)
//        return 0;
//    
//    int t = round(log2(abs(dt * fac)));
//    
//    if(t > 7) 
//        t = 7;
//        
//    if(dt < 0)
//        t = -t;
//        
//    return t;     
//}

//#define FIRSTCHAR   ' '
//#define CHARHEIGHT  7
//
//char widthtab[] = {
//    0x05,0x00,0x00,0x00,0x00,0x05,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x03,0x01,0x00,
//    0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,
//    0x00,0x00,0x00,0x03 
//};
//
//char chartab[] = {
//    0x00,0x00,0x00,0x00,0x00,           // SPACE
//    0x44,0x20,0x10,0x08,0x44,           // %
//    0x08,0x14,0x08,                     // '                
//    0x00,0x00,0x00,                     // +                
//    0x08,0x08,0x08,                     // -                
//    0x40,                               // .                          
//    0x3e,0x51,0x49,0x45,0x3e,           // 0
//    0x00,0x04,0x02,0x7f,0x00,           // 1
//    0x62,0x51,0x49,0x49,0x46,           // 2
//    0x22,0x41,0x49,0x49,0x36,           // 3
//    0x07,0x08,0x08,0x08,0x76,           // 4
//    0x2f,0x49,0x49,0x49,0x31,           // 5
//    0x3e,0x49,0x49,0x49,0x32,           // 6
//    0x00,0x01,0x71,0x09,0x07,           // 7
//    0x36,0x49,0x49,0x49,0x36,           // 8
//    0x26,0x49,0x49,0x49,0x3e,           // 9
//    0x38,0x44,0x44                      // C
//};