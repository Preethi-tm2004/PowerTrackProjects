#include "tftp.h"
#include "udp.h"

#include <sys/time.h>

int udp_bind_server(void)
{
    int sockfd;
    struct sockaddr_in server_addr;

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    /* Clear address structure */
    memset(&server_addr, 0, sizeof(server_addr));

    /* IPv4 */
    server_addr.sin_family = AF_INET;

    /* Accept packets on any local interface */
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* TFTP server port */
    server_addr.sin_port = htons(TFTP_PORT);

    /* Bind socket to IP + port */
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int udp_rebind_server(void)
{
    int sockfd;
    struct sockaddr_in server_addr;

    /* Create new UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* let OS choose an available port */
    server_addr.sin_port = htons(0);

    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int udp_bind_client(void)
{
    int sockfd;
    struct sockaddr_in client_addr;

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    /* Clear address structure */
    memset(&client_addr, 0, sizeof(client_addr));

    /* IPv4 */
    client_addr.sin_family = AF_INET;

    /* Accept any local interface */
    client_addr.sin_addr.s_addr = INADDR_ANY;

    /* Let OS choose an available port */
    client_addr.sin_port = htons(0);

    /* Bind clinet socket */
    if(bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int udp_send_packet(int sockfd, const void *buffer, size_t length, const struct sockaddr_in *destination)
{
    int bytes_sent;

    bytes_sent = sendto(sockfd, buffer, length, 0, (struct sockaddr *)destination, sizeof(*destination));

    if(bytes_sent < 0)
    {
        perror("sendto");
        return -1;
    }

    return bytes_sent;
}

int udp_receive_packet(int sockfd, void *buffer, size_t buffer_size, struct sockaddr_in *source)
{
    int bytes_received;
    socklen_t source_len;

    source_len = sizeof(*source);

    bytes_received = recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)source, &source_len);

    if(bytes_received < 0)
    {
        perror("recvfrom");
        return -1;
    }

    return bytes_received;
}

int udp_set_timeout(int sockfd, int seconds)
{
    struct timeval timeout;

    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt");
        return -1;
    }

    return 0;
}

