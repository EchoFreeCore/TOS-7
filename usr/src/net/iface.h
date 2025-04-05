#ifndef NET_IFACE_H
#define NET_IFACE_H

#include <stdint.h>

#define IF_NAME_MAX 16

typedef struct net_if {
    char name[IF_NAME_MAX];
    uint32_t ip_addr;
    uint32_t netmask;
    int (*send)(struct net_if *, const void *, size_t);
} net_if_t;

void iface_init(void);
void if_register(net_if_t *iface);
net_if_t *if_lookup(const char *name);

// loopback handler
int iface_loopback_send(net_if_t *iface, const void *data, size_t len);

#endif