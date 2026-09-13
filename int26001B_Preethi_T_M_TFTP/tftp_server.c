#include "file.h"
#include "tftp.h"
#include "udp.h"

int handle_wrq(struct sockaddr_in *client_addr, tftp_packet_t *request);
int handle_rrq(struct sockaddr_in *client_addr, tftp_packet_t *request);

int main(void)
{
    int server_sockfd;
    unsigned char buffer[PACKET_SIZE];
    struct sockaddr_in client_addr;
    int bytes_received;
    tftp_packet_t packet;
    
    /* Create and bind server socket */
    server_sockfd = udp_bind_server();

    if(server_sockfd < 0)
    {
        return 1;
    }

    printf("TFTP server started on port %d\n", TFTP_PORT);

    while(1)
    {
        bytes_received = udp_receive_packet(server_sockfd, buffer, sizeof(buffer), &client_addr);

        if(bytes_received < 0)
        {
            continue;
        }

        if(packet_parse(buffer, bytes_received, &packet) < 0)
        {
            printf("Invalid TFTP packet received\n");
            continue;
        }

        printf("Received opcode: %u\n", packet.opcode);

        switch(packet.opcode)
        {
            case OP_RRQ:
            {
                printf("Received RRQ for file: %s\n", packet.filename);

                if(handle_rrq(&client_addr, &packet) < 0)
                {
                    printf("RRQ transfer failed\n");
                }
                break;
            }

            case OP_WRQ:
            {
                printf("Received WRQ for file: %s\n", packet.filename);

                if(handle_wrq(&client_addr, &packet) < 0)
                {
                    printf("WRQ transfer failed\n");
                }
                break;
            }

            default:
                printf("Unknown opcode: %u\n", packet.opcode);
                break;
        }
    }

    close(server_sockfd);

    return 0;
}

int handle_wrq(struct sockaddr_in *client_addr, tftp_packet_t *request)
{
    int fd;
    int transfer_sockfd;
    uint8_t ack_buffer[PACKET_SIZE];
    uint8_t buffer[PACKET_SIZE];
    
    int ack_length; 
    int bytes_received;
    int retries = 0;

    uint16_t expected_block = INITIAL_BLOCK;

    tftp_packet_t packet;

    /* Open the requested file for writing */
    fd = file_open_write(request->filename);

    if(fd < 0)
    {
        printf("Failed to open file for writing: %s\n",
               request->filename);
        return -1;
    }

    /* Use a new socket for this file transfer */
    transfer_sockfd = udp_rebind_server();

    if(transfer_sockfd < 0)
    {
        file_close(fd);
        return -1;
    }

    /* Send ACK 0 to confirm the WRQ */
    ack_length = packet_form_ack(ack_buffer, sizeof(ack_buffer), INITIAL_ACK_BLOCK);

    if(ack_length < 0)
    {
        close(transfer_sockfd);
        file_close(fd);
        return -1;
    }

    if(udp_send_packet(transfer_sockfd, ack_buffer, ack_length, client_addr) < 0)
    {
        close(transfer_sockfd);
        file_close(fd);
        return -1;
    }

    /* Set timeout so lost DATA packets can be retransmitted */
    if(udp_set_timeout(transfer_sockfd, TFTP_TIMEOUT_SEC) < 0)
    {
        close(transfer_sockfd);
        file_close(fd);
        return -1;
    }

    while(1)
    {
        bytes_received = udp_receive_packet(transfer_sockfd, buffer, sizeof(buffer), client_addr);

        if(bytes_received < 0)
        {
            retries++;

            if(retries >= TFTP_MAX_RETRIES)
            {
                printf("Maximum retries reached\n");
                close(transfer_sockfd);
                file_close(fd);
                return -1;
            }

            /* Retransmit the previous ACK when DATA is lost */
            if(udp_send_packet(transfer_sockfd, ack_buffer, ack_length, client_addr) < 0)
            {
                close(transfer_sockfd);
                file_close(fd);
                return -1;
            }

            continue;
        }

        retries = 0;

        if(packet_parse(buffer, bytes_received, &packet) < 0)
        {
            printf("Invalid TFTP packet received\n");
            continue;
        }

        if(packet_receive_error(&packet) == 0)
        {
            printf("TFTP Error %u: %s\n", packet.error_code, packet.error_message);

            close(transfer_sockfd);
            file_close(fd);
            return -1;
        }

        if(packet.opcode != OP_DATA)
        {
            printf("Expected DATA packet\n");
            continue;
        }

        if(packet_receive_data(&packet) < 0)
        {
            printf("Invalid DATA packet\n");
            continue;
        }

        /* Accept only the DATA block we are expecting */
        if(packet.blocknum != expected_block)
        {
            printf("Unexpected block number: %u\n", packet.blocknum);
            continue;
        }

        /* Write received DATA into the destination file */
        if(file_write(fd, packet.data, packet.data_length) < 0)
        {
            printf("Failed to write file\n");
            close(transfer_sockfd);
            file_close(fd);
            return -1;
        }

        /* Acknowledge the received DATA block */
        ack_length = packet_form_ack(ack_buffer, sizeof(ack_buffer), packet.blocknum);

        if(ack_length < 0)
        {
            close(transfer_sockfd);
            file_close(fd);
            return -1;
        }

        if(udp_send_packet(transfer_sockfd, ack_buffer, ack_length, client_addr) < 0)
        {
            close(transfer_sockfd);
            file_close(fd);
            return -1;
        }

        printf("Received block %u (%d bytes)\n", packet.blocknum, packet.data_length);

        /* Last block is identified by less than 512 bytes */
        if(packet.data_length < DATA_SIZE)
        {
            break;
        }

        expected_block++;
    }

    file_close(fd);
    close(transfer_sockfd);

    printf("File received successfully: %s\n", request->filename);

    return 0;
}

int handle_rrq(struct sockaddr_in *client_addr, tftp_packet_t *request)
{
    int fd;
    int transfer_sockfd;
    uint8_t data_buffer[DATA_SIZE];
    uint8_t data_packet[PACKET_SIZE];
    uint8_t buffer[PACKET_SIZE];

    int bytes_read;
    int data_packet_length;
    int bytes_received;

    int retries;
    uint16_t blocknum = INITIAL_BLOCK;

    tftp_packet_t packet;

    /* Open the requested file for reading */
    fd = file_open_read(request->filename);

    if(fd < 0)
    {
        printf("Failed to open file: %s\n", request->filename);
        return -1;
    }

    /* Use a new socket for this file transfer */
    transfer_sockfd = udp_rebind_server();

    if(transfer_sockfd < 0)
    {
        file_close(fd);
        return -1;
    }

    /* Set timeout for ACK reception */
    if(udp_set_timeout(transfer_sockfd, TFTP_TIMEOUT_SEC) < 0)
    {
        close(transfer_sockfd);
        file_close(fd);
        return -1;
    }

    while(1)
    {
        /* Read the next 512-byte block from the file */
        bytes_read = file_read(fd, data_buffer, DATA_SIZE);

        if(bytes_read < 0)
        {
            printf("Failed to read file\n");
            close(transfer_sockfd);
            file_close(fd);
            return -1;
        }

        /* Create DATA packet with the current block number */
        data_packet_length = packet_form_data(data_packet, sizeof(data_packet), blocknum, (const char *)data_buffer, bytes_read);

        if(data_packet_length < 0)
        {
            printf("Failed to create DATA packet\n");
            close(transfer_sockfd);
            file_close(fd);
            return -1;
        }

        retries = 0;

        while(1)
        {
            /* Send DATA and wait for its ACK */
            if(udp_send_packet(transfer_sockfd, data_packet, data_packet_length, client_addr) < 0)
            {
                close(transfer_sockfd);
                file_close(fd);
                return -1;
            }

            bytes_received = udp_receive_packet(transfer_sockfd, buffer, sizeof(buffer), client_addr);

            if(bytes_received < 0)
            {
                retries++;

                if(retries >= TFTP_MAX_RETRIES)
                {
                    printf("Maximum retries reached\n");
                    close(transfer_sockfd);
                    file_close(fd);
                    return -1;
                }

                printf("Timeout, retransmitting DATA block %u...\n", blocknum);

                continue;
            }

            retries = 0;

            if(packet_parse(buffer, bytes_received, &packet) < 0)
            {
                printf("Invalid TFTP packet received\n");
                continue;
            }

            if(packet_receive_error(&packet) == 0)
            {
                printf("TFTP Error %u: %s\n", packet.error_code, packet.error_message);

                close(transfer_sockfd);
                file_close(fd);
                return -1;
            }

            if(packet_receive_ack(&packet) < 0)
            {
                printf("Expected ACK packet\n");
                continue;
            }

            if(packet.blocknum != blocknum)
            {
                printf("Unexpected ACK block number: %u\n", packet.blocknum);
                continue;
            }

            break;
        }

        printf("Sent block %u (%d bytes)\n", blocknum, bytes_read);

        /* Less than 512 bytes means this is the final block */
        if(bytes_read < DATA_SIZE)
        {
            break;
        }

        blocknum++;
    }

    file_close(fd);
    close(transfer_sockfd);

    printf("File sent successfully: %s\n", request->filename);

    return 0;
}