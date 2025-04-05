#ifndef NET_UDP_H
#define NET_UDP_H

#include <stdint.h>
#include <stddef.h>
#include "net/iface.h"

typedef void (*udp_recv_fn)(uint32_t src_ip, uint16_t src_port,
                            uint32_t dst_ip,
                            const void *payload, size_t len);

void udp_init(void);
int udp_bind(uint16_t port, udp_recv_fn handler);
void udp_send(net_if_t *iface,
              uint16_t src_port, uint16_t dst_port,
              uint32_t dst_ip,
              const void *payload, size_t len);
void udp_input(const void *data, size_t len,
               uint32_t src_ip, uint32_t dst_ip);

#endif