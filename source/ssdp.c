#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "socket.h"
#include "ssdp.h"
#include "mem.h"
#include "common.h"

static int ssdp_loop = 1;

int ssdp_seek(struct ssdp_list_t **ssdp_list) 
{
	struct sockaddr_in addr;
	char message[BUFFER_SIZE] = {'\0'};
	char header[BUFFER_SIZE] = {'\0'};
	int sock, match = 0;
	int timeout = 1;
	ssize_t len = 0;
	socklen_t addrlen = sizeof(addr);
	unsigned short int nip[4], port = 0;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout * 100000;

	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("error: could not create ssdp socket\n");
		goto end;
	}

	memset((void *)&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1900);
	addr.sin_addr.s_addr = inet_addr("239.255.255.250");

	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

	strcpy(header, "M-SEARCH * HTTP/1.1\r\n"
				   "Host:239.255.255.250:1900\r\n"
				   "ST:urn:schemas-upnp-org:service:pilight:1\r\n"
				   "Man:\"ssdp:discover\"\r\n"
				   "MX:3\r\n\r\n");

	if((len = sendto(sock, header, BUFFER_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr))) >= 0){
		// printf("ssdp sent search\n");
    }

	int x = 0;
	
	while(ssdp_loop){
		memset(message, '\0', BUFFER_SIZE);
		
		if((x = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *)&addr, &addrlen)) < 1){
			//perror("read");
			goto end;
		}

		if(strstr(message, "pilight") > 0){
			char **array = NULL;
			unsigned int n = explode(message, "\r\n", &array), q = 0;
			
			for(q=0; q<n; q++)
				if(match==0 && sscanf(array[q], "Location:%hu.%hu.%hu.%hu:%hu\r\n", &nip[0], &nip[1], &nip[2], &nip[3], &port)>0)
					match = 1;
			
			array_free(&array, n);
			
			if(match == 1){
				struct ssdp_list_t *node = MALLOC(sizeof(struct ssdp_list_t));
				
				if(node == NULL){
					printf("error: out of memory\n");
					exit(EXIT_FAILURE);
				}
				
				sprintf(node->ip, "%hu.%hu.%hu.%hu", nip[0], nip[1], nip[2], nip[3]);
				node->ip[16] = '\0';
				node->port = port;
				node->next = *ssdp_list;
				*ssdp_list = node;
			}
		}
	}
	
	goto end;

end:
	if(sock > 0)
		close(sock);
	
	struct ssdp_list_t *ptr = *ssdp_list, *next = NULL, *prev = NULL;
	
	if(match == 1){
		while(ptr){
			next = ptr->next;
			ptr->next = prev;
			prev = ptr;
			ptr = next;
		}
		
		if(ptr != NULL)
			FREE(ptr);
		
		if(next != NULL)
			FREE(next);
		
		*ssdp_list = prev;

		return 0;
	} 
	else{
		return -1;
	}
}

void ssdp_free(struct ssdp_list_t *ssdp_list) 
{
	struct ssdp_list_t *tmp = NULL;
	
	while(ssdp_list){
		tmp = ssdp_list;
		ssdp_list = ssdp_list->next;
		FREE(tmp);
	}
	
	if(ssdp_list != NULL)
		FREE(ssdp_list);
}