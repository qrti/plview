#ifndef _SSDP_H_
#define _SSDP_H_

typedef struct ssdp_list_t{
    char ip[17];
    unsigned short port;
    struct ssdp_list_t *next;
}ssdp_list_t;

int ssdp_seek(struct ssdp_list_t **ssdp_list);
void ssdp_free(struct ssdp_list_t *ssdp_list);

#endif
