/*
 * syscalls.c
 *
 * Minimal implementation av systemanrop för nätverksstacken.
 * Dessa anrop binder användarutrymmet (via POSIX-sockets) till den interna
 * nätverksstacken. Inspirerat av Unix V7‑andan – enkelt, direkt och robust.
 *
 * Implementerade anrop:
 *   sys_socket()  : Skapar ett socket (endast AF_INET, SOCK_DGRAM stöds).
 *   sys_bind()    : Binder socket till en lokal port.
 *   sys_sendto()  : Skickar data via socket.
 *   sys_recvfrom(): Mottar data (ej implementerat, returnerar fel).
 */

 #include "syscalls.h"
 #include "socket.h"   // Minimal intern representation av socket (se nedan)
 #include "udp.h"
 #include "kmalloc.h"
 #include "net_stack.h"
 #include "iface.h"
 #include <stddef.h>
 #include <string.h>
 
 /* Minimal socketstruktur för UDP. I en riktig implementation
    skulle detta vara en del av ett filhanteringssystem. */
 typedef struct {
     int domain;
     int type;
     int protocol;
     int bound;
     uint16_t local_port;  // Porten i nätverksordning
 } socket_t;
 
 #define MAX_SOCKETS 64
 
 static socket_t *socket_table[MAX_SOCKETS] = {0};
 
 /*
  * Hjälpfunktion: Returnerar standardnätverksinterface.
  * Här antas att vi använder det första registrerade interfacet (ex. lo0 eller eth0).
  */
 static net_if_t *get_default_net_if(void) {
     // En verklig implementation bör välja ett interface baserat på konfiguration.
     extern net_if_t *if_list[];  // Om if_list är exponerad, annars implementera en getter.
     extern int if_count;
     return (if_count > 0) ? if_list[0] : NULL;
 }
 
 /*
  * sys_socket()
  *
  * Skapar ett nytt socket. Endast AF_INET och SOCK_DGRAM stöds.
  * Returnerar ett socket-ID (index i socket_table) vid framgång, annars -1.
  */
 int sys_socket(int domain, int type, int protocol) {
     if (domain != AF_INET || type != SOCK_DGRAM)
         return -1;
 
     for (int i = 0; i < MAX_SOCKETS; i++) {
         if (socket_table[i] == 0) {
             socket_t *sock = (socket_t *)kmalloc(sizeof(socket_t));
             if (!sock)
                 return -1;
             sock->domain = domain;
             sock->type = type;
             sock->protocol = protocol;
             sock->bound = 0;
             sock->local_port = 0;
             socket_table[i] = sock;
             return i; // Socket-ID
         }
     }
     return -1; // Inga lediga sockets
 }
 
 /*
  * sys_bind()
  *
  * Binder ett socket till en lokal adress (sockaddr_in).
  */
 int sys_bind(int sockfd, const struct sockaddr_in *addr, size_t addrlen) {
     if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == 0)
         return -1;
     if (addrlen < sizeof(struct sockaddr_in))
         return -1;
 
     socket_t *sock = socket_table[sockfd];
     sock->local_port = addr->sin_port; // Porten i nätverksordning
     sock->bound = 1;
     return 0;
 }
 
 /*
  * sys_sendto()
  *
  * Skickar data via socket. Skickar data genom UDP-lagret.
  */
 ssize_t sys_sendto(int sockfd, const void *buf, size_t len,
                    int flags, const struct sockaddr_in *dest_addr, size_t addrlen) {
     if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == 0)
         return -1;
     if (addrlen < sizeof(struct sockaddr_in))
         return -1;
 
     socket_t *sock = socket_table[sockfd];
     if (!sock->bound)
         return -1;  // Socketen måste vara bunden till en lokal port
 
     net_if_t *iface = get_default_net_if();
     if (!iface)
         return -1;
 
     udp_send(iface, sock->local_port, dest_addr->sin_port,
              dest_addr->sin_addr.s_addr, buf, len);
     return len;
 }
 
 /*
  * sys_recvfrom()
  *
  * Tar emot data via socket. I denna förenklade version
  * returneras -1 då mottagningslogiken inte är implementerad.
  */
 ssize_t sys_recvfrom(int sockfd, void *buf, size_t len,
                      int flags, struct sockaddr_in *src_addr, size_t *addrlen) {
     (void)sockfd;
     (void)buf;
     (void)len;
     (void)flags;
     (void)src_addr;
     (void)addrlen;
     // En fullständig implementation skulle hantera interna socket-buffertar.
     return -1;
 }
 