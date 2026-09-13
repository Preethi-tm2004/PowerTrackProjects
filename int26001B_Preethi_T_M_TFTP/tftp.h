#ifndef TFTP_H
#define TFTP_H

#include <stdint.h>
#include <stddef.h>

/* TFTP uses UDP port 69 for the initial request[TFTP's well-known initial server port is UDP 69] */
#define TFTP_PORT 69

/* TFTP Packet size */
#define DATA_SIZE 512
#define PACKET_SIZE 516

/* Mximum sizes used by our application */
#define MAX_FILENAME        255
#define MAX_MODE            20
#define MAX_ERROR_MESSAGE   256

/* Block numbering */
#define INITIAL_BLOCK       1
#define INITIAL_ACK_BLOCK   0

/* Timeout and retry */
#define TFTP_TIMEOUT_SEC    3
#define TFTP_MAX_RETRIES    5

/* Operation code */
#define OP_RRQ      1
#define OP_WRQ      2
#define OP_DATA     3
#define OP_ACK      4
#define OP_ERROR    5

/* TFTP Modes */
#define MODE_OCTET      "octet"
#define MODE_NETASCII   "netascii"

/* TFTP Error codes */
#define ERR_NOT_DEFINED         0
#define ERR_FILE_NOT_FOUND      1
#define ERR_ACCESS_VIOLATION    2
#define ERR_DISK_FULL           3
#define ERR_ILLEGAL_OP          4
#define ERR_UNKNOWN_TID         5
#define ERR_FILE_EXISTS         6
#define ERR_NO_SUCH_USER        7


/* TFTP packet's structure */
typedef struct 
{
    char filename[MAX_FILENAME + 1];
    uint16_t opcode;
    char mode[MAX_MODE];
    
    char data[DATA_SIZE];
    int data_length;

    uint16_t blocknum;

    uint16_t error_code;
    char error_message[MAX_ERROR_MESSAGE];
    
}tftp_packet_t;

/* Packet parsing */
int packet_parse(const uint8_t *buffer, size_t length, tftp_packet_t *packet);

int packet_extract_opcode(const uint8_t *buffer, size_t length, uint16_t *opcode);

int packet_parse_rq(const uint8_t *buffer, size_t length, tftp_packet_t *packet);

int packet_parse_data(const uint8_t *buffer, size_t length, tftp_packet_t *packet);

int packet_parse_ack(const uint8_t *buffer, size_t length, tftp_packet_t *packet);

int packet_parse_error(const uint8_t *buffer, size_t length, tftp_packet_t *packet);

/* Packet formation */
int packet_form_rrq(uint8_t *buffer, size_t buffer_size, const char *filename, const char *mode);

int packet_form_wrq(uint8_t *buffer, size_t buffer_size, const char *filename, const char *mode);

int packet_form_data(uint8_t *buffer, size_t buffer_size, uint16_t blocknum, const char *data, size_t data_length);

int packet_form_ack(uint8_t *buffer, size_t buffer_size, uint16_t blocknum);

int packet_form_error(uint8_t *buffer,  size_t buffer_size, uint16_t error_code, const char *message);

/* Packet Reciept */
int packet_receive_rrq(const tftp_packet_t *packet);

int packet_receive_wrq(const tftp_packet_t *packet);

int packet_receive_data(const tftp_packet_t *packet);

int packet_receive_ack(const tftp_packet_t *packet);

int packet_receive_error(const tftp_packet_t *packet);

int packet_receive_invalid(const tftp_packet_t *packet);

#endif