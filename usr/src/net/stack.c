#include "net/stack.h"
#include "net/iface.h"
#include "net/ip.h"
#include "net/udp.h"
#include "net/raw.h"
#include "net/packet.h"

void net_stack_init(void) {
    packet_pool_init();
    iface_init();         // lo0, eth0 m.m.
    udp_init();           // registrerar UDP-logic
    raw_init();           // registrerar RAW logic
}

void net_stack_rx(net_if_t *iface, const void *data, size_t len) {
    ip_input(iface, data, len);  // vi hanterar bara IPv4 just nu
}
