#include "tftp.h"
#include "udp.h"
#include "file.h"

static void print_menu(void)
{
    printf("\n\033[32mCommands:\033[0m\n");
    printf("  \033[34mconnect <IP> [port]        Connect to server\033[0m\n");
    printf("  \033[34mget <filename>             Receive a file from server\033[0m\n");
    printf("  \033[34mput <filename>             Send a file to server\033[0m\n");
    printf("  \033[34mmode <octet|netascii>      Set transfer mode\033[0m\n");
    printf("  \033[34mbye / quit                 Close and exit\033[0m\n");
    printf("  \033[34mhelp                       Show this menu\033[0m\n\n");
}

int tftp_connect(const char *ip, int *sockfd, struct sockaddr_in *server_addr)
{
    if(ip == NULL || sockfd == NULL || server_addr == NULL)
        return -1;

    /* Initialize the structure before filling it */
    memset(server_addr, 0, sizeof(*server_addr));

    /* Validate server IP address */
    if(inet_pton(AF_INET, ip, &server_addr->sin_addr) != 1)
    {
        printf("Invalid IP address\n");
        return -1;
    }

    /* Create and bind clinet UDP socket */
    *sockfd = udp_bind_client();

    if(*sockfd < 0)
    {
        printf("Failed to create clinet socket\n");
        return -1;
    }

    /* Configure server address */
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(TFTP_PORT);

    return 0;
}

int tftp_get(int sockfd, struct  sockaddr_in *server_addr, const char *filename, const char *mode)
{
    uint8_t buffer[PACKET_SIZE];
    int packet_length;
    int retries = 0;
    uint8_t rrq_buffer[PACKET_SIZE];
    uint8_t ack_buffer[PACKET_SIZE];

    /* Create RRQ packets */
    packet_length = packet_form_rrq(rrq_buffer, sizeof(rrq_buffer), filename, mode);

    if(packet_length < 0)
    {
        printf("Failed to create RRQ packet\n");
        return -1;
    }

    int fd;
    /* Open the destination file */
    fd = file_open_write(filename);

    if(fd < 0)
    {
        printf("Failed to open file for writing\n");
        return -1;
    }

    /* Send RRQ to server */
    if(udp_send_packet(sockfd, rrq_buffer, packet_length, server_addr) < 0)
    {
        printf("Failed to send RRQ\n");
        return -1;
    }

    if(udp_set_timeout(sockfd, TFTP_TIMEOUT_SEC) < 0)
    {
        printf("Failed to set socket timeout\n");
        file_close(fd);
        return -1;
    }

    tftp_packet_t packet;
    int bytes_received;
    uint16_t expected_block = INITIAL_BLOCK;
    int first_data_received = 0;
    int ack_length;

    while(1)
    {
        /* Receive the DATA packet */
        bytes_received = udp_receive_packet(sockfd, buffer, sizeof(buffer), server_addr);

        if(bytes_received < 0)
        {
            retries++;

            if(retries >= TFTP_MAX_RETRIES)
            {
                printf("Maximum retries reached\n");
                file_close(fd);
                return -1;
            }
            
            printf("Timeout, retrying...\n");

            if(first_data_received == 0)
            {
                /* First DATA not received, retransmit RRQ */
                if(udp_send_packet(sockfd, rrq_buffer, packet_length, server_addr) < 0)
                {
                    printf("Failed to retransmit RRQ\n");
                    file_close(fd);
                    return -1;
                }
            }
            else
            {
                /* Later DATA not received, retransmit previous ACK */
                if(udp_send_packet(sockfd, ack_buffer, ack_length, server_addr) < 0)
                {
                    printf("Failed to retransmit ACK\n");
                    file_close(fd);
                    return -1;
                }
            }

            continue;
        }

        /* DATA received successfully */
        retries = 0;

        /* Parse the packet */
        if(packet_parse(buffer, bytes_received, &packet) < 0)
        {
            printf("Invalid TFTP packet received\n");
            return -1;
        }

        /* Check for server errors */
        if(packet_receive_error(&packet) == 0)
        {
            printf("TFTP Error %u: %s\n", packet.error_code, packet.error_message);

            file_close(fd);
            return -1;
        }

        if(packet.opcode != OP_DATA)
        {
            printf("Expected DATA packet\n");
            file_close(fd);
            return -1;
        }

        /* Validate that it is a data */
        if(packet_receive_data(&packet) < 0)
        {
            printf("Expected DATA packet\n");
            file_close(fd);
            return -1;
        }

        first_data_received = 1;

        if(packet.blocknum != expected_block)
        {
            printf("Unexpected block number\n");
            file_close(fd);
            return -1;
        }

        /* Write the first DATA block */
        if(file_write(fd, packet.data, packet.data_length) < 0)
        {
            printf("Failed to write file\n");
            file_close(fd);
            return -1;
        }

        /* Send the acknowledgement for block 1 */
        ack_length = packet_form_ack(ack_buffer, sizeof(ack_buffer), packet.blocknum);

        if(ack_length < 0)
        {
            printf("Failed to create ACK packet\n");
            file_close(fd);
            return -1;
        }

        if(udp_send_packet(sockfd, ack_buffer, ack_length, server_addr) < 0)
        {
            printf("Failed to send ACK\n");
            file_close(fd);
            return -1;
        }
        expected_block++;

        if(packet.data_length < DATA_SIZE)
        {
            break;
        }
    }

    file_close(fd);
    return 0;
}

int tftp_put(int sockfd, struct sockaddr_in *server_addr, const char *filename, const char *mode)
{
    int fd;
    uint8_t buffer[PACKET_SIZE];

    uint8_t data_buffer[DATA_SIZE];
    ssize_t bytes_read;

    uint16_t blocknum = INITIAL_BLOCK;

    uint8_t wrq_buffer[PACKET_SIZE];
    int wrq_length;

    uint8_t data_packet[PACKET_SIZE];
    int data_packet_length;

    int retries = 0;

    /* Open the source file */
    fd = file_open_read(filename);

    if(fd < 0)
    {
        printf("Failed to open file for reading\n");
        return -1;
    }

    /* Create WRQ packet */
    wrq_length = packet_form_wrq(wrq_buffer, sizeof(wrq_buffer), filename, mode);

    if(wrq_length < 0)
    {
        printf("Failed to create WRQ packet\n");
        file_close(fd);
        return -1;
    }

    /* Set socket timeout */
    if(udp_set_timeout(sockfd, TFTP_TIMEOUT_SEC) < 0)
    {
        printf("Failed to set socket timeout\n");
        file_close(fd);
        return -1;
    }

    tftp_packet_t packet;
    int bytes_received;

    /* Wait for ACK #0 */
    while(1)
    {
        /* Send WRQ to server */
        if(udp_send_packet(sockfd, wrq_buffer, wrq_length, server_addr) < 0)
        {
            printf("Failed to send WRQ\n");
            file_close(fd);
            return -1;
        }

        /* Receive ACK #0 */
        bytes_received = udp_receive_packet(sockfd, buffer, sizeof(buffer), server_addr);

        if(bytes_received < 0)
        {
            retries++; 

            if(retries >= TFTP_MAX_RETRIES)
            {
                printf("Maximum retries reached\n");
                file_close(fd);
                return -1;
            }

            printf("Timeout, retrying WRQ...\n");

            continue;
        }
        /* ACK received successfully */
        retries = 0;
        break;
    }

    if(packet_parse(buffer, bytes_received, &packet) < 0)
    {
        printf("Invalid TFTP packet received\n");
        file_close(fd);
        return -1;
    }

    /* Check if it is an acknowledgement */
    if(packet_receive_ack(&packet) < 0)
    {
        printf("Expected ACK packet\n");
        file_close(fd);
        return -1;
    }

    /* Verify that it's an acknowledgement block */
    if(packet.blocknum != INITIAL_ACK_BLOCK)
    {
        printf("Unexpected ACK block number\n");
        file_close(fd);
        return -1;
    }

    while(1)
    {
        /* Read the file */
        bytes_read = file_read(fd, data_buffer, DATA_SIZE);

        if(bytes_read < 0)
        {
            printf("Failed to read file\n");
            file_close(fd);
            return -1;
        }

        /* Create the DATA packet */
        data_packet_length = packet_form_data(data_packet, sizeof(data_packet), blocknum, (const char *)data_buffer, bytes_read);

        if(data_packet_length < 0)
        {
            printf("Failed to create DATA packet\n");
            file_close(fd);
            return -1;
        }

        /* Reset retry count for this DATA packet */
        retries = 0;

        /* Send DATA and wait for ACK */
        while(1)
        {
            /* Send the DATA packet */
            if(udp_send_packet(sockfd, data_packet,
                            data_packet_length, server_addr) < 0)
            {
                printf("Failed to send DATA packet\n");
                file_close(fd);
                return -1;
            }

            /* Receive the acknowledgement */
            bytes_received = udp_receive_packet(sockfd, buffer, sizeof(buffer), server_addr);

            if(bytes_received < 0)
            {
                retries++;

                if(retries >= TFTP_MAX_RETRIES)
                {
                    printf("Maximum retries reached\n");
                    file_close(fd);
                    return -1;
                }

                printf("Timeout, retransmitting DATA block %u...\n", blocknum);
                continue;
            }

            /* ACK received successfully */
            retries = 0;
            break;
        }

        if(packet_parse(buffer, bytes_received, &packet) < 0)
        {
            printf("Invalid TFTP packet received\n");
            file_close(fd);
            return -1;
        }

        if(packet_receive_error(&packet) == 0)
        {
            printf("TFTP Error %u: %s\n",
                packet.error_code,
                packet.error_message);

            file_close(fd);
            return -1;
        }

        /* Validate the acknowledgement */
        if(packet_receive_ack(&packet) < 0)
        {
            printf("Expected ACK packet\n");
            file_close(fd);
            return -1;
        }

        /* Verify the block number */
        if(packet.blocknum != blocknum)
        {
            printf("Unexpected ACK block number\n");
            file_close(fd);
            return -1;
        }

        if(bytes_read < DATA_SIZE)
        {
            break;
        }

        blocknum++;
    }

    file_close(fd);
    return 0;
}

int main(void)
{
    int sockfd = -1;
    int connected = 0;

    struct sockaddr_in server_addr;

    char command[100];
    char argument[256];

    char mode[MAX_MODE] = MODE_OCTET;

    print_menu();

    while(1)
    {
        printf("tftp> ");

        if(fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }

        /* Remove newline from the entered command */
        command[strcspn(command, "\n")] = '\0';

        if(strcmp(command, "help") == 0)
        {
            print_menu();
        }
        else if(strcmp(command, "bye") == 0 || strcmp(command, "quit") == 0)
        {
            /* Close the client socket before exiting */
            if(connected)
            {
                close(sockfd);
            }

            printf("\033[32mGoodbye!\033[0m\n");
            break;
        }
        else if(sscanf(command, "connect %255s", argument) == 1)
        {
            /* Connect to the specified TFTP server */
            if(connected)
            {
                close(sockfd);
                connected = 0;
            }

            if(tftp_connect(argument, &sockfd, &server_addr) == 0)
            {
                connected = 1;

                /* Use the project testing port */
                server_addr.sin_port = htons(TFTP_PORT);

                printf("\033[32mConnected to %s:%d\033[0m\n", argument, TFTP_PORT);
            }
            else
            {
                printf("\033[31mConnection failed\033[0m\n");
            }
        }
        else if(sscanf(command, "mode %255s", argument) == 1)
        {
            /* Change the transfer mode used in RRQ and WRQ */
            if(strcmp(argument, MODE_OCTET) == 0 || strcmp(argument, MODE_NETASCII) == 0)
            {
                strcpy(mode, argument);
                printf("\033[32mTransfer mode set to %s\033[0m\n", mode);
            }
            else
            {
                printf("\033[31mInvalid mode. Use octet or netascii\033[0m\n");
            }
        }
        else if(sscanf(command, "get %255s", argument) == 1)
        {
            if(!connected)
            {
                printf("\033[31mPlease connect to a server first\033[0m\n");
                continue;
            }

            /* Download the requested file from the server */
            if(tftp_get(sockfd, &server_addr, argument, mode) == 0)
            {
                printf("\033[32mGET successful\033[0m\n");
            }
            else
            {
                printf("\033[31mGET failed\033[0m\n");
            }
        }
        else if(sscanf(command, "put %255s", argument) == 1)
        {
            if(!connected)
            {
                printf("\033[31mPlease connect to a server first\033[0m\n");
                continue;
            }

            /* Upload the requested file to the server */
            if(tftp_put(sockfd, &server_addr, argument, mode) == 0)
            {
                printf("\033[32mPUT successful\033[0m\n");
            }
            else
            {
                printf("\033[31mPUT failed\033[0m\n");
            }
        }
        else if(command[0] == '\0')
        {
            continue;
        }
        else
        {
            printf("\033[31mUnknown command. Type 'help' for available commands.\033[0m\n");
        }
    }

    return 0;
}

