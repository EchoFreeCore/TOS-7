#ifndef NET_PACKET_H
#define NET_PACKET_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PACKET_SIZE 1536

typedef struct net_packet {
    uint32_t src_ip, dst_ip;
    uint16_t src_port, dst_port;
    size_t len;
    uint8_t *data;
    struct net_packet *next;
} net_packet_t;

void packet_pool_init(void);
net_packet_t *packet_alloc(void);
void packet_free(net_packet_t *pkt);

#endif
