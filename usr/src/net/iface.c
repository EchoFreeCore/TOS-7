#include "net/iface.h"
#include "net/stack.h"
#include <string.h>

static net_if_t lo0;

void iface_init(void) {
    memset(&lo0, 0, sizeof(lo0));
    strcpy(lo0.name, "lo0");
    lo0.ip_addr = 0x7F000001;     // 127.0.0.1
    lo0.netmask = 0xFF000000;
    lo0.send = iface_loopback_send;
    if_register(&lo0);
}

int iface_loopback_send(net_if_t *iface, const void *data, size_t len) {
    void *copy = kmalloc(len);
    if (!copy) return -1;
    memcpy(copy, data, len);
    net_stack_rx(iface, copy, len);
    kfree(copy);
    return 0;
}

#define MAX_IFACES 4
static net_if_t *if_list[MAX_IFACES];
static int if_count = 0;

void if_register(net_if_t *iface) {
    if (if_count < MAX_IFACES)
        if_list[if_count++] = iface;
}

net_if_t *if_lookup(const char *name) {
    for (int i = 0; i < if_count; i++)
        if (strcmp(if_list[i]->name, name) == 0)
            return if_list[i];
    return NULL;
}
