#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "socket.h"
#include "ssdp.h"
#include "json.h"
#include "mem.h"
#include "gc.h"
#include "display.h"
#include "ini.h"

extern Station* station;            // from ini.c
extern uint16_t stations;

static int main_loop = 1;
static char *recvBuff = NULL;
static int sockfd = 0;

int main_gc(void)
{
    main_loop = 0;
    sleep(1);

    if(recvBuff != NULL){
        FREE(recvBuff);
        recvBuff = NULL;
    }

    if(sockfd > 0){
        socket_write(sockfd, "HEART");
        socket_close(sockfd);
    }

    xfree();
    return 0;
}

int main(int argc, char **argv)
{
    unsigned short port = 0;
    char* server = NULL;
    unsigned short stats = 0;

    struct ssdp_list_t *ssdp_list = NULL;
    
    atomicinit();
    gc_attach(main_gc);
    gc_catch();

    if(server != NULL && port > 0) {
        if((sockfd = socket_connect(server, port)) == -1){
            printf("error: could not connect to pilight-daemon\n");
            return EXIT_FAILURE;
        }
    }
    else if(ssdp_seek(&ssdp_list) == -1){
        printf("error: no pilight ssdp connections found\n");
        goto close;
    }
    else{
        if((sockfd = socket_connect(ssdp_list->ip, ssdp_list->port)) == -1){
            printf("error: could not connect to pilight-daemon\n");
            goto close;
        }
    }

    if(ssdp_list != NULL)
        ssdp_free(ssdp_list);

    if(server != NULL)
        FREE(server);

    struct JsonNode *jclient = json_mkobject();
    struct JsonNode *joptions = json_mkobject();
    json_append_member(jclient, "action", json_mkstring("identify"));
    json_append_member(joptions, "receiver", json_mknumber(1, 0));
    json_append_member(joptions, "stats", json_mknumber(stats, 0));
    json_append_member(jclient, "options", joptions);
    char *out = json_stringify(jclient, NULL);
    socket_write(sockfd, out);
    json_free(out);
    json_delete(jclient);

    if(socket_read(sockfd, &recvBuff, 0)!=0 || strcmp(recvBuff, "{\"status\":\"success\"}")!=0)
        goto close;
        
    if(iniInit("config.ini"))                       // get values from config.ini
        goto close;

    display(0, 0, 0);                               // init display

    unsigned short filteropt = 1;
    int lastmin = -1, firstrx = 0;

    while(main_loop){
        if(socket_read(sockfd, &recvBuff, 0) != 0)
            goto close;

        char **array = NULL;
        unsigned int n = explode(recvBuff, "\n", &array), i = 0;

        for(i=0; i<n; i++){
            struct JsonNode *jcontent = json_decode(array[i]);
            struct JsonNode *jtype = json_find_member(jcontent, "type");

            if(jtype != NULL){
                json_remove_from_parent(jtype);
                json_delete(jtype);
            }

            if(filteropt == 1){
                char *pr = NULL;
                double id = 0.0;
                double ch = 0.0;
                
                struct JsonNode *jmessage = json_find_member(jcontent, "message");
                json_find_string(jcontent, "protocol", &pr);
                json_find_number(jmessage, "id", &id);
                json_find_number(jmessage, "channel", &ch);                
                int j = 0;

                for(j=0; j<stations; j++){                                                          // step through protocol filters
                    if(strcmp(station[j].pr, "-")==0 || strcmp(station[j].pr, pr)==0){              // protocol not specified or found
                        if(j == 0){                                                                 // datetime protocol is the first
                            if(firstrx){
                                double min = 0.0;
                                json_find_number(jmessage, "minute", &min); 

                                if((int)min != lastmin){
                                    display(-1, 0.0, 0.0);  
                                    lastmin = (int)min;
                                }                      
                            }                          
                        }                            
                        else if(strcmp(station[j].id, "-")==0 || (int)id==atoi(station[j].id)){     // id not specified or found
                            if(strcmp(station[j].ch, "-")==0  || (int)ch==atoi(station[j].ch)){     // channel not specified or found
                                double temp = 0.0, humi = 0.0;
                                json_find_number(jmessage, "temperature", &temp);
                                json_find_number(jmessage, "humidity", &humi);
                                display(j-1, temp, humi);      
                                firstrx = 1;  
                                break;          
                            }
                        }
                    }
                }
            }
            else{
                char *content = json_stringify(jcontent, "\t");
                printf("%s\n", content);
                json_free(content);
            }

            json_delete(jcontent);
        }

        array_free(&array, n);        
    }

close:
    if(sockfd > 0)
        socket_close(sockfd);

    if(recvBuff != NULL){
        FREE(recvBuff);
        recvBuff = NULL;
    }

    iniClean();

    return EXIT_SUCCESS;
}
