#include "net/packet.h"
#include <string.h>
#include "kmalloc.h"

#define POOL_SIZE 128

static net_packet_t pool[POOL_SIZE];
static uint8_t payload_pool[POOL_SIZE][MAX_PACKET_SIZE];
static net_packet_t *free_list;

void packet_pool_init(void) {
    free_list = NULL;
    for (int i = 0; i < POOL_SIZE; i++) {
        pool[i].data = payload_pool[i];
        pool[i].next = free_list;
        free_list = &pool[i];
    }
}

net_packet_t *packet_alloc(void) {
    if (!free_list) return NULL;
    net_packet_t *pkt = free_list;
    free_list = pkt->next;
    pkt->len = 0;
    return pkt;
}

void packet_free(net_packet_t *pkt) {
    pkt->next = free_list;
    free_list = pkt;
}
