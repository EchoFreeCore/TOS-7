#include "net/udp.h"
#include "net/ip.h"
#include "net/packet.h"
#include <string.h>
#include <stdlib.h>

#define MAX_UDP_SOCKETS 16

typedef struct {
    uint16_t port;
    udp_recv_fn on_recv;
} udp_sock_t;

static udp_sock_t udp_sockets[MAX_UDP_SOCKETS];

void udp_init(void) {
    memset(udp_sockets, 0, sizeof(udp_sockets));
}

int udp_bind(uint16_t port, udp_recv_fn handler) {
    for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
        if (udp_sockets[i].port == 0) {
            udp_sockets[i].port = port;
            udp_sockets[i].on_recv = handler;
            return 0;
        }
    }
    return -1;
}

void udp_input(const void *data, size_t len,
               uint32_t src_ip, uint32_t dst_ip) {
    if (len < 8) return;

    const uint8_t *d = data;
    uint16_t src_port = (d[0] << 8) | d[1];
    uint16_t dst_port = (d[2] << 8) | d[3];
    size_t payload_len = len - 8;

    for (int i = 0; i < MAX_UDP_SOCKETS; i++) {
        if (udp_sockets[i].port == dst_port) {
            udp_sockets[i].on_recv(src_ip, src_port, dst_ip,
                                   d + 8, payload_len);
            return;
        }
    }
}

void udp_send(net_if_t *iface,
              uint16_t src_port, uint16_t dst_port,
              uint32_t dst_ip,
              const void *payload, size_t len) {
    uint8_t *packet = kmalloc(8 + len);
    if (!packet) return;

    packet[0] = (src_port >> 8) & 0xFF;
    packet[1] = (src_port >> 0) & 0xFF;
    packet[2] = (dst_port >> 8) & 0xFF;
    packet[3] = (dst_port >> 0) & 0xFF;
    packet[4] = ((8 + len) >> 8) & 0xFF;
    packet[5] = ((8 + len) >> 0) & 0xFF;
    packet[6] = 0x00; packet[7] = 0x00;

    memcpy(packet + 8, payload, len);
    ip_output(iface, 17, packet, 8 + len, dst_ip);
    kfree(packet);
}
