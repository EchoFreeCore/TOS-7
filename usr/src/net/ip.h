#ifndef NET_IP_H
#define NET_IP_H

#include <stdint.h>
#include <stddef.h>
#include "net/iface.h"

void ip_input(net_if_t *iface, const void *data, size_t len);
void ip_output(net_if_t *iface, uint8_t proto,
               const void *payload, size_t len,
               uint32_t dst_ip);

#endif