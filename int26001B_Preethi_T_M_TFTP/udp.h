#ifndef UDP_H
#define UDP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* Bind server socket to UDP port 69*/
int udp_bind_server(void);

/* Bind server to new transfer port, if needed */
int udp_rebind_server(void);

/* Bind client socket to an available local port */
int udp_bind_client(void);

/* Send raw TFTP bytes using sendto */
int udp_send_packet(int sockfd, const void *buffer, size_t length, const struct sockaddr_in *destination);

/* Receive raw bytes using recvfrom */
int udp_receive_packet(int sockfd, void *buffer, size_t buffer_size, struct sockaddr_in *source);

/* Set receive timeout */
int udp_set_timeout(int sockfd, int seconds);

#endif


