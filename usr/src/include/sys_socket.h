#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Skapar ett nytt socket. Endast AF_INET och SOCK_DGRAM stöds i detta exempel. */
int sys_socket(int domain, int type, int protocol);

/* Binder socket till en lokal adress (sockaddr_in). */
int sys_bind(int sockfd, const struct sockaddr_in *addr, size_t addrlen);

/* Skickar data via socket till en destination (sockaddr_in). */
ssize_t sys_sendto(int sockfd, const void *buf, size_t len,
                   int flags, const struct sockaddr_in *dest_addr, size_t addrlen);

/* Tar emot data via socket. Förenklad – returnerar -1 då mottagningslogiken ej är implementerad. */
ssize_t sys_recvfrom(int sockfd, void *buf, size_t len,
                     int flags, struct sockaddr_in *src_addr, size_t *addrlen);

#ifdef __cplusplus
}
#endif

#endif /* SYSCALLS_H */
