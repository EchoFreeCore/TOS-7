#ifndef NET_STACK_H
#define NET_STACK_H

#include <stddef.h>
struct net_if;

void net_stack_init(void);
void net_stack_rx(struct net_if *iface, const void *data, size_t len);

#endif
