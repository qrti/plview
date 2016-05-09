#ifndef _SOCKET_H_
#define _SOCKET_H_

#define EOSS                "\n\n"
#define BUFFER_SIZE         1025
#define MAX_CLIENTS			30

int socket_connect(char*, unsigned short);
int socket_write(int, const char*, ...);
int socket_read(int, char**, time_t);
void socket_close(int);

#endif
