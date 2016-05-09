// ini.c

#include "ini.h"

#define SECTIONS            2
#define SECT_NUMSTATIONS    0
#define SECT_STATION        1

char const* section[SECTIONS] = { "NUMSTATIONS", "STATION" };
char data[MAXDATA + 1];         // +1 for termination if used as string
             
Station* station;               // for plview.c
uint16_t stations;                         
                               
FILE* file=0;                               
uint16_t sect=0;
int16_t dc=-1;
int16_t lsc=0;                        
bool warning=false;                 
                                
void iniClean()
{
    if(file) fclose(file);    
    
    if(station){
        Station* s = station;
        
        for(uint16_t i=0; i<stations; i++){
            if(s->pr) free(s->pr);
            if(s->id) free(s->id);
            if(s->ch) free(s->ch);
            s++;
        }
        
        free(station);
    }
}

bool iniInit(char const* name)
{
    if((file=fopen(name, "r")) == 0){
        printf("error: cannot open %s\n", name);
        return 1;
    }
    
    if(iniRead()){
        printf("error: wrong format in %s\n", name);
        return 1;        
    }

    fclose(file);    
    file = 0;

    if(warning)
        sleep(3);
    
    return 0;
}

bool iniRead()
{
    char line[MAXLINE];
    uint16_t lc = 1;
    
    while(fgets(line, MAXLINE, file)){
        char* p = line;
        iniStrip(p);
        p = iniTrim(p);        
        
        if(strlen(p)){
            if(*p == '['){
                if(iniAllocData())
                    return true;
                
                lsc = lc;
                
                if(iniReadSection(p))
                    return true;
            }
            else{
                if(iniReadValues(p))
                    return true;
            }
        }
        
        lc++;
    };    
    
    if(dc)
        iniAllocData();
    
    return false;
}

bool iniReadSection(char* p)
{
    char* v;
    
    if((v = strchr(++p, ']'))){ 
        *v = '\0';
    }
    else{
        printf("error: invalid ini section [%s <-\n", p);
        return true;
    }
    
    for(sect=0; sect<SECTIONS; sect++)
        if(strcmp(section[sect], p) == 0)
            break;
    
    if(sect == SECTIONS){
        printf("warning: unknown ini section: [%s]\n", p);        
        warning = true;
    }
    
    return false;
}

bool iniReadValues(char* p)
{
    char* v = strtok(p, ",");

    while(v){
        if(dc >= MAXDATA){                                  // if more data on next line
            printf("error: ini section too long\n");
            return true;            
        }         
        
        v = iniTrim(v);                                     // iniStrip leading and trailing spaces
        
        if(v[0] == '\''){                                   // character literal
            if(v[2] != '\''){
                printf("error: invalid ini char\n");
                return true;                
            }
            
            data[dc++] = v[1];
        }
        else if(v[0]=='0' && v[1]=='x'){                    // hexadecimal
            data[dc++] = strtol(v, NULL, 16);
        }
        else if(v[0] == '\"'){                              // string
            uint16_t len = strlen(v);
            
            if(v[len-1] != '\"'){            
                printf("error: invalid ini string\n");
                return true;                   
            }
            
            len -= 2;                                       // iniStrip quotation marks
            
            if(len+dc > MAXDATA){                           
                printf("error: ini string too long\n");
                return true;
            }
            
            for(int i=0; i<len; i++) 
                data[dc++] = v[i+1];
        }
        else{                                               // digits
            data[dc++] = atoi(v);
        }
        
        v = strtok(NULL, ",");                              // get next token
    }
        
    return false;
}

bool iniAllocData()
{
    static uint16_t sc;
    
    if(dc >= 0){
        if(sect == SECT_NUMSTATIONS){
            if(dc != 1){
                printf("error: invalid section [%s]\n", section[sect]);
                return true;
            }
            
            stations = data[0];
            
            if(stations > MAXSTATIONS){
                printf("error: max %d statiins allowed in [%s]\n", MAXSTATIONS, section[sect]);
                return true;            
            }
            
            station = (Station*)malloc(stations * sizeof(Station));
        }
        else if(sect == SECT_STATION){
            if(!station){
                printf("error: [NUMSTATIONS] must be defined before [STATION]\n");
                return true;                    
            }
            
            if(sc >= stations){
                printf("warning: station %d ignored\n", sc++);
                warning = true;
                dc = 0;
                return false;
            }
            
            if(dc < 3){
                printf("warning: invalid station %d ignored\n", sc);
                warning = true;
                dc = 0;
                return false;                
            }
            
            char* pr = data;
            uint16_t prlen = strlen(pr);
            
            if(prlen==0 || prlen>MAXFLDLEN){
                printf("warning: invalid station %d ignored\n", sc);
                warning = true;
                dc = 0;
                return false;                 
            }
            
            char* id = pr + prlen + 1;
            uint16_t idlen = strlen(id);

            if(idlen==0 || idlen>MAXFLDLEN){
                printf("warning: invalid station %d ignored\n", sc);
                warning = true;
                dc = 0;
                return false;                 
            }

            char* ch = id + idlen + 1;
            uint16_t chlen = strlen(ch);

            if(chlen==0 || chlen>MAXFLDLEN){
                printf("warning: invalid station %d ignored\n", sc);
                warning = true;
                dc = 0;
                return false;                 
            }
            
            char* p = (char*)malloc(prlen + 1);
            memcpy(p, pr, prlen + 1);
            (station + sc)->pr = p;
            
            p = (char*)malloc(idlen + 1);
            memcpy(p, id, idlen + 1);
            (station + sc)->id = p;            

            p = (char*)malloc(chlen + 1);
            memcpy(p, ch, chlen + 1);
            (station + sc)->ch = p;          
            
            sc++;    
        }
    }
    
    dc = 0;
    return false;
}

void iniStrip(char* p)
{
    char* v = p; 
    
    while(*p != '\0'){                  // iniStrip comments  
        if(*p=='/' && *(p+1)=='/'){
            *p = '\0';
            break;
        }
        
        p++;
    }
    
    while(*v != '\0'){                  // tabs to single spaces  
        if(*v=='\t' || *v=='\r' || *v=='\n')
            *v = ' ';    
        
        v++;    
    }        
}

char* iniTrim(char* p)
{
    while(*p == ' ')                    // iniTrim leading spaces
        p++;
    
    if(*p == '\0')             
        return p;
    
    char* e = p + strlen(p) - 1;        // iniTrim trailing spaces CR and LF
    
    while(e>p && *e==' ')    
        e--;
    
    *(e + 1) = '\0';    
        
    return p;    
}
    