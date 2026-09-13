#include "tftp.h"

#include <arpa/inet.h>
#include <string.h>

int packet_extract_opcode(const uint8_t *buffer, size_t length, uint16_t *opcode)
{
    uint16_t value;
    
    if(buffer == NULL || opcode == NULL)
    {
        return -1;
    }

    if(length < 2)
    {
        return -1;
    }

    /* copy the first 2 bytes from the buffer into the value */
    memcpy(&value, buffer, sizeof(uint16_t));

    /* Convert a 16-bit value from network byte order->host byte order */
    *opcode = ntohs(value);

    return 0;
}

int packet_parse(const uint8_t *buffer, size_t length, tftp_packet_t *packet)
{
    uint16_t opcode; //temporarily stores OPCODE

    if(buffer == NULL || packet == NULL)
    {
        return -1;
    }

    if(packet_extract_opcode(buffer, length, &opcode) < 0) //extract opcode
    {
        return -1;
    }

    packet->opcode = opcode;

    /* decides which parser should handle the packet */
    switch(opcode)
    {
        case OP_RRQ:
        case OP_WRQ:
            return packet_parse_rq(buffer, length, packet);

        case OP_DATA:
            return packet_parse_data(buffer, length, packet);

        case OP_ACK:
            return packet_parse_ack(buffer, length, packet);

        case OP_ERROR:
            return packet_parse_error(buffer, length, packet);

        /* for invalid opcode */
        default:
            return -1;
    }
}

/*
 * Parse an RRQ or WRQ packet.
 * Packet format:
 * +--------+----------+------+--------+------+
 * | Opcode | Filename |  0   |  Mode  |  0   |
 * +--------+----------+------+--------+------+
 * Opcode:
 *     1 -> RRQ
 *     2 -> WRQ
 */
int packet_parse_rq(const uint8_t *buffer, size_t length, tftp_packet_t *packet)
{
    size_t position;
    size_t filename_length;
    size_t mode_length;

    const uint8_t *filename_end;
    const uint8_t *mode_end;

    /* Check for invalid pointers */
    if(buffer == NULL || packet == NULL)
    {
        return -1;
    }

    /* 
    * The smallest valid RRQ/WRQ packet contains:
    * 2 bytes -> opcode, 1 byte  -> filename, 1 byte  -> filename NULL terminator, 1 byte  -> mode, 1 byte  -> mode NULL terminator
    * Therefore minimum length is 6 bytes.
    */
    if (length < 6)
        return -1;

    position = 2; //filename starts after opcode(2bytes)
   
    /*
     * Search for the NULL character that marks
     * the end of the filename.
     * memchr() searches only within the remaining
     * bytes of the received packet.
     */
    filename_end = memchr(&buffer[position], '\0', length - position);

    /* NULL terminator was not found */
    if(filename_end == NULL)
        return -1;

    /*  filename_end points to the NULL character. Subtracting the starting address gives the number of filename characters.*/
    filename_length = filename_end - &buffer[position];

    /* Make sure filename fits inside our internal packet structure */
    if(filename_length > MAX_FILENAME)
        return -1;

    /* Copy filename along with NULL terminator into our packet structure */
    memcpy(packet->filename, &buffer[position], filename_length + 1);

    /* Move past the filename and its terminator to reach the mode */
    position = position + filename_length  + 1;

    /* The mode must contain at least 1 byte and must be terminated by NULL */
    if(position >= length)
        return -1;

     /* Search for the NULL character that marks the end of the mode */
    mode_end = memchr(&buffer[position], '\0', length - position);

    /* NULL terminator was not found */
    if (mode_end == NULL)
        return -1;

    /* Find the NULL character that marks the end of the mode */
    mode_length = mode_end - &buffer[position];

    /* Make sure the mode fits inside the internal structure */
    if(mode_length >= MAX_MODE)
        return -1;

    /* Copy the mode into our packet structure */
    memcpy(packet->mode, &buffer[position], mode_length + 1);

    return 0;
}

/*
 * Parse a DATA packet.
 * Packet format:
 * +--------+--------+----------------------+
 * | Opcode | Block  |        Data          |
 * +--------+--------+----------------------+
 * | 2 bytes| 2 bytes|      0-512 bytes     |
 * +--------+--------+----------------------+
 */
int packet_parse_data(const uint8_t *buffer, size_t length, tftp_packet_t *packet)
{
    uint16_t blocknum;

    /* Check for invalid pointer */
    if(buffer == NULL || packet == NULL)
        return -1;

    /* Data packet must contain 2 bytes->opcode, 2 bytes->block number */
    if(length < 4)
        return -1;

    /* extract the block number, block number is stored in network byte order 
    *  so first copy the 2 bytes and then convert to host byte order */
    memcpy(&blocknum, &buffer[2], sizeof(uint16_t));

    packet->blocknum = ntohs(blocknum);

    /* Data starts after opcode[2 bytes] and block number[2 bytes]*/
    packet->data_length = length -4;

    /* data should not exceeds maximum TFTP DATA playload size */
    if(packet->data_length > DATA_SIZE)
        return -1;

    /* Copy the actual file data into our packet structure */
    if(packet->data_length > 0)
    {
        memcpy(packet->data, &buffer[4], packet->data_length);
    }

    return 0;

}

/*
 * Parse an ACK packet.
 * Packet format:
 * +--------+--------+
 * | Opcode | Block  |
 * +--------+--------+
 * | 2 bytes| 2 bytes|
 * +--------+--------+
 */
int packet_parse_ack(const uint8_t *buffer, size_t length, tftp_packet_t *packet)
{
    uint16_t blocknum;

    /* Check for invalid pointers */
    if(buffer == NULL || packet == NULL)
        return -1;

    /* An acknowledge packet must contain 2bytes->opcode, 2bytes->block number */
    if(length != 4)
        return -1;

    /* block number starts after 2bytes of opcode */
    memcpy(&blocknum, &buffer[2], sizeof(uint16_t));

    packet->blocknum = ntohs(blocknum);

    return 0;
}

/*
 * Parse an ERROR packet.
 * Format:
 * +--------+--------+----------------+------+
 * | Opcode | ECode  | Error Message  |  0   |
 * +--------+--------+----------------+------+
 */
int packet_parse_error(const uint8_t *buffer, size_t length, tftp_packet_t *packet)
{
    uint16_t error_code;

    size_t message_length;

    const uint8_t *message_end;

    /* Check pointers */
    if(buffer == NULL || packet == NULL)
        return -1;

    /* Error packets needs atleast 5 bytes */
    if(length < 5)
        return -1;

    /* Copy the error code */
    memcpy(&error_code, &buffer[2], sizeof(uint16_t));

    /* Convert network byte to host system */
    packet->error_code = ntohs(error_code);

    message_end = memchr(&buffer[4], '\0', length - 4);

    /* Message must have NULL terminator */
    if(message_end == NULL)
        return -1;

    message_length = message_end - &buffer[4];

    if(message_length >= MAX_ERROR_MESSAGE)
        return -1;

    /* Copy error message */
    memcpy(packet->error_message, &buffer[4], message_length + 1);

    return 0;
}

/* Create an RRQ packet */
int packet_form_rrq(uint8_t *buffer, size_t buffer_size, const char *filename, const char *mode)
{
    uint16_t opcode;

    size_t filename_length;
    size_t mode_length;
    size_t position;

    if(buffer == NULL || filename == NULL || mode == NULL)
        return -1;

    /* string length */
    filename_length = strlen(filename);
    mode_length = strlen(mode);

    /* Check filename size */
    if(filename_length > MAX_FILENAME)
        return -1;

    /* Check mode size */
    if(mode_length >= MAX_MODE)
        return -1;
        
    /* Total packet size */
    if(buffer_size < 2 + filename_length + 1 + mode_length + 1)
        return -1;

    opcode = htons(OP_RRQ);

    /* Copy opcode into buffer */
    memcpy(buffer, &opcode, sizeof(uint16_t));

    /* starts after opcode */
    position = 2;

    /* Copy filename including NULL */
    memcpy(&buffer[position], filename, filename_length + 1);

    /* go to mode */
    position = position + filename_length + 1;

    /* Copy mode including NULL */
    memcpy(&buffer[position], mode, mode_length + 1);

    /* return actual packet size */
    return position + mode_length + 1;
}

/* Create an RRQ packet */
int packet_form_wrq(uint8_t *buffer, size_t buffer_size, const char *filename, const char *mode)
{
    uint16_t opcode;

    size_t filename_length;
    size_t mode_length;
    size_t position;

    if(buffer == NULL || filename == NULL || mode == NULL)
        return -1;

    /* string length */
    filename_length = strlen(filename);
    mode_length = strlen(mode);

    /* Check filename size */
    if(filename_length > MAX_FILENAME)
        return -1;

    /* Check mode size */
    if(mode_length >= MAX_MODE)
        return -1;
        
    /* Total packet size */
    if(buffer_size < 2 + filename_length + 1 + mode_length + 1)
        return -1;

    opcode = htons(OP_WRQ);

    /* Copy opcode into buffer */
    memcpy(buffer, &opcode, sizeof(uint16_t));

    /* starts after opcode */
    position = 2;

    /* Copy filename including NULL */
    memcpy(&buffer[position], filename, filename_length + 1);

    /* go to mode */
    position = position + filename_length + 1;

    /* Copy mode including NULL */
    memcpy(&buffer[position], mode, mode_length + 1);

    /* return actual packet size */
    return position + mode_length + 1;
}

/* Create a Data Packet */
int packet_form_data(uint8_t *buffer, size_t buffer_size, uint16_t blocknum, const char *data, size_t data_length)
{
    uint16_t opcode;
    uint16_t block;
    
    size_t position;

    /* check buffer and data pointers */
    if(buffer == NULL || (data == NULL &&  data_length > 0))
        return -1;

    /* Data should not contain more that 512 bytes */
    if(data_length > DATA_SIZE)
        return -1;

    /* Check buffer size */
    if(buffer_size < 4 + data_length)
        return -1;

    opcode = htons(OP_DATA);

    memcpy(buffer, &opcode, sizeof(uint16_t));

    block = htons(blocknum);

    /* copy block number into buffer */
    memcpy(&buffer[2], &block, sizeof(uint16_t));

    /* Position starts after opcode and block number */
    position = 4;

    /* Copy data if present */
    if(data_length > 0)
    {
        memcpy(&buffer[position], data, data_length);
    }

    return position + data_length; 
}

/* Create an ACK Packet */
int packet_form_ack(uint8_t *buffer, size_t buffer_size, uint16_t blocknum)
{
    uint16_t opcode;
    uint16_t block;

    if(buffer == NULL)
        return -1;

    /* ACK packet needs 4 bytes */
    if(buffer_size < 4)
        return -1;

    /* Convert opcode to network byte order */
    opcode = htons(OP_ACK);

    /* Convert block number to network byte order */
    block = htons(blocknum);

    /* Copy opcode */
    memcpy(buffer, &opcode, sizeof(uint16_t));

    /* Copy block */
    memcpy(&buffer[2], &block, sizeof(uint16_t));

    /* Return packet size */
    return 4;
}

/* Create an ERROR packet */
int packet_form_error(uint8_t *buffer, size_t buffer_size, uint16_t error_code, const char *message)
{
    uint16_t opcode;
    uint16_t code;

    size_t message_length;
    size_t position;

    if(buffer == NULL || message == NULL)
        return -1;

    /* Find message length */
    message_length = strlen(message);

    /*Check message size */
    if(message_length >= MAX_ERROR_MESSAGE)
        return -1;

    /* Check total packet size 
    *  2 bytes -> opcode, 2 bytes -> error code, message_length -> message, 1 byte -> NULL */
    if(buffer_size < 4 + message_length + 1)
        return -1;

    /* Convert opcode to network byte order */
    opcode = htons(OP_ERROR);

    /* Convert error code to network byte order */
    code = htons(error_code);

    /* Copy opcode */
    memcpy(buffer, &opcode, sizeof(uint16_t));

    /* Copy error code */
    memcpy(&buffer[2], &code, sizeof(uint16_t));

    /* Message starts after opcode and error code */
    position = 4;

    /* Copy error message including NULL */
    memcpy(&buffer[position], message, message_length + 1);

    /* Return actual packet size */
    return position + message_length + 1;  
}

/* ----------------- Verify the packets ------------------ */
int packet_receive_rrq(const tftp_packet_t *packet)
{
    if(packet == NULL)
        return -1;

    if(packet->opcode != OP_RRQ)
        return -1;

    return 0;
}

int packet_receive_wrq(const tftp_packet_t *packet)
{
    if(packet == NULL)
        return -1;

    if(packet->opcode != OP_WRQ)
        return -1;

    return 0;
}

int packet_receive_data(const tftp_packet_t *packet)
{
    if(packet == NULL)
        return -1;

    if(packet->opcode != OP_DATA)
        return -1;

    return 0;
}

int packet_receive_ack(const tftp_packet_t *packet)
{
    if(packet == NULL)
        return -1;

    if(packet->opcode != OP_ACK)
        return -1;

    return 0;
}

int packet_receive_error(const tftp_packet_t *packet)
{
    if(packet == NULL)
        return -1;

    if(packet->opcode != OP_ERROR)
        return -1;

    return 0;
}




