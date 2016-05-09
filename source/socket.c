#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "socket.h"
#include "mem.h"

static char recvBuff[BUFFER_SIZE];
static int socket_clients[MAX_CLIENTS];
static unsigned short socket_loop = 1;

int socket_connect(char* address, unsigned short port)
{
    struct sockaddr_in serv_addr;
    int sockfd;
    fd_set fdset;
    struct timeval tv;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("error: could not create socket\n");
        return -1;
    }

    memset(&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    int tmpres = inet_pton(AF_INET, address, &serv_addr.sin_addr);

    if(tmpres < 0){
        printf("error: can't set s_addr\n");
        return -1;
    }
    else if(tmpres == 0){
        printf("error: %s is not a valid IP address\n", address);
        return -1;
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1){
        fcntl(sockfd, F_SETFL, O_NONBLOCK);

        FD_ZERO(&fdset);
        FD_SET((long unsigned int)sockfd, &fdset);
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        if(select(sockfd+1, NULL, &fdset, NULL, &tv) == 1){
            int error = -1;
            socklen_t len = sizeof(error);
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

            if(error == 0){
                return sockfd;
            }
            else{
                close(sockfd);
                return -1;
            }
        }
        else{
            close(sockfd);
            return -1;
        }
    }
    else{
        printf("error: connect\n");
        close(sockfd);
        return -1;
    }
}

int socket_write(int sockfd, const char* msg, ...) 
{
    va_list ap;
    int bytes = -1;
    int ptr = 0, n = 0, x = BUFFER_SIZE, len = (int)strlen(EOSS);
    char *sendBuff = NULL;
    
    if(strlen(msg) > 0 && sockfd > 0){
        va_start(ap, msg);
        n = vsnprintf(NULL, 0, msg, ap);
        
        if(n == -1){
            printf("error: improperly formatted string: %s\n", msg);
            return -1;
        }
        
        n += (int)len;
        va_end(ap);

        if((sendBuff = MALLOC((size_t)n)) == NULL){
            printf("error: out of memory\n");
            exit(EXIT_FAILURE);
        }
        
        memset(sendBuff, '\0', (size_t)n);

        va_start(ap, msg);
        vsprintf(sendBuff, msg, ap);
        va_end(ap);

        memcpy(&sendBuff[n-len], EOSS, (size_t)len);

        while(ptr < n){
            if((n-ptr) < BUFFER_SIZE){
                x = (n-ptr);
            } 
            else{
                x = BUFFER_SIZE;
            }
            
            if((bytes = (int)send(sockfd, &sendBuff[ptr], (size_t)x, MSG_NOSIGNAL)) == -1){
                sendBuff[n-(len-1)] = '\0';             // change the delimiter into regular newlines
                sendBuff[n-(len)] = '\n';
                printf("error: socket write failed: %s\n", sendBuff);
                FREE(sendBuff);
                return -1;
            }
            
            ptr += bytes;
        }

        if(strncmp(&sendBuff[0], "BEAT", 4) != 0){
            sendBuff[n-(len-1)] = '\0';                 // change the delimiter into regular newlines
            sendBuff[n-(len)] = '\n';
        }
        
        FREE(sendBuff);
    }
    
    return n;
}

int socket_read(int sockfd, char **message, time_t timeout) 
{
    struct timeval tv;
    int bytes = 0;
    size_t msglen = 0;
    int ptr = 0, n = 0, len = (int)strlen(EOSS);
    fd_set fdsread;
    
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    if(timeout > 0){
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
    }

    while(socket_loop && sockfd > 0){
        FD_ZERO(&fdsread);
        FD_SET((unsigned long)sockfd, &fdsread);

        do{            
            if(timeout > 0)
                n = select(sockfd+1, &fdsread, NULL, NULL, &tv);
            else
                n = select(sockfd+1, &fdsread, NULL, NULL, 0);
        }while(n == -1 && errno == EINTR && socket_loop);
        
        if(timeout > 0 && n == 0)
            return 1;
        
        if(socket_loop == 0)        // immediatly stop loop if the select was waken up by the garbage collector
            break;
        
        if(n == -1){
            return -1;
        } 
        else if(n > 0){            
            if(FD_ISSET((unsigned long)sockfd, &fdsread)){
                bytes = (int)recv(sockfd, recvBuff, BUFFER_SIZE, 0);

                if(bytes <= 0){
                    return -1;
                } 
                else{
                    ptr+=bytes;
                    
                    if((*message = REALLOC(*message, (size_t)ptr+1)) == NULL){
                        printf("error: out of memory\n");
                        exit(EXIT_FAILURE);
                    }
                    
                    memset(&(*message)[(ptr-bytes)], '\0', (size_t)bytes+1);
                    memcpy(&(*message)[(ptr-bytes)], recvBuff, (size_t)bytes);
                    msglen = strlen(*message);
                }
                
                if(*message && msglen > 0){
                    int l = 0;
                    
                    if(((l = strncmp(&(*message)[ptr-(len)], EOSS, (unsigned int)(len))) == 0) || ptr < BUFFER_SIZE){
                        if(ptr > msglen){
                            int i = 0;
                            
                            for(i=0; i<ptr; i++){
                                if(i+(len-1) < ptr && strncmp(&(*message)[i], EOSS, (size_t)len) == 0){
                                    memmove(&(*message)[i], message[i+(len-1)], (size_t)(ptr-(i+(len-1))));
                                    ptr-=(len-1);
                                    (*message)[i] = '\n';
                                }
                            }
                            
                            (*message)[ptr] = '\0';
                        } 
                        else{
                            if(l == 0)
                                (*message)[ptr-(len)] = '\0';       // remove delimiter
                            else
                                (*message)[ptr] = '\0';
                            
                            if(strcmp(*message, "1") == 0 || strcmp(*message, "BEAT") == 0)
                                return -1;
                        }
                        
                        return 0;
                    }
                }
            }
        }
    }

    return -1;
}

void socket_close(int sockfd) 
{
    int i = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buf[INET_ADDRSTRLEN + 1];

    if(sockfd > 0){
        if(getpeername(sockfd, (struct sockaddr*)&address, (socklen_t*)&addrlen) == 0){
            memset(&buf, '\0', INET_ADDRSTRLEN + 1);
            inet_ntop(AF_INET, (void *)&(address.sin_addr), buf, INET_ADDRSTRLEN + 1);
            printf("client disconnected, ip %s, port %d\n", buf, ntohs(address.sin_port));
        }

        for(i=0; i<MAX_CLIENTS; i++){
            if(socket_clients[i] == sockfd){
                socket_clients[i] = 0;
                break;
            }
        }
        
        shutdown(sockfd, 2);
        close(sockfd);
    }
}
