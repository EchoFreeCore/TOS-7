#include "net/ip.h"
#include "net/udp.h"
#include "net/raw.h"
#include <string.h>
#include <stdlib.h>
#include "net/packet.h"

void ip_input(net_if_t *iface, const void *data, size_t len) {
    if (len < 20) return;

    const uint8_t *pkt = data;
    uint8_t version = pkt[0] >> 4;
    uint8_t ihl = pkt[0] & 0x0F;
    if (version != 4 || ihl < 5) return;

    size_t header_len = ihl * 4;
    if (len < header_len) return;

    uint8_t proto = pkt[9];
    uint32_t src_ip = (pkt[12]<<24) | (pkt[13]<<16) | (pkt[14]<<8) | pkt[15];
    uint32_t dst_ip = (pkt[16]<<24) | (pkt[17]<<16) | (pkt[18]<<8) | pkt[19];

    const void *payload = pkt + header_len;
    size_t payload_len = len - header_len;

    switch (proto) {
        case 17: udp_input(payload, payload_len, src_ip, dst_ip); break;
        default: raw_input(proto, payload, payload_len, src_ip, dst_ip); break;
    }
}

void ip_output(net_if_t *iface, uint8_t proto,
               const void *payload, size_t len,
               uint32_t dst_ip) {
    if (!iface || !iface->send || len > 1480) return;

    size_t total_len = 20 + len;
    uint8_t *packet = kmalloc(total_len);
    if (!packet) return;

    memset(packet, 0, 20);
    packet[0] = 0x45;                  // IPv4, IHL=5
    packet[1] = 0x00;
    packet[2] = (total_len >> 8) & 0xFF;
    packet[3] = (total_len >> 0) & 0xFF;
    packet[4] = 0x00; packet[5] = 0x01; // ID
    packet[6] = 0x00; packet[7] = 0x00;
    packet[8] = 64;                    // TTL
    packet[9] = proto;

    uint32_t src_ip = iface->ip_addr;
    packet[12] = (src_ip >> 24) & 0xFF;
    packet[13] = (src_ip >> 16) & 0xFF;
    packet[14] = (src_ip >> 8) & 0xFF;
    packet[15] = (src_ip >> 0) & 0xFF;

    packet[16] = (dst_ip >> 24) & 0xFF;
    packet[17] = (dst_ip >> 16) & 0xFF;
    packet[18] = (dst_ip >> 8) & 0xFF;
    packet[19] = (dst_ip >> 0) & 0xFF;

    memcpy(packet + 20, payload, len);
    iface->send(iface, packet, total_len);
    kfree(packet);
}
