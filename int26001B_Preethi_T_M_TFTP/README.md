# TFTP Client-Server

**Name:** Preethi T M  
**Batch:** int26001B  
**Project:** Trivial File Transfer Protocol (TFTP)

## Project Description

Implemented a TFTP client-server application using C and UDP sockets.
The project supports file upload (PUT/WRQ) and file download (GET/RRQ)
using TFTP packets with DATA and ACK mechanisms, block-based file
transfer, timeout and retransmission handling, and transfer modes
such as octet and netascii.

## Build

gcc -Wall -Wextra tftp_client.c udp.c tftp.c file.c -o tftp_client

gcc -Wall -Wextra tftp_server.c udp.c tftp.c file.c -o tftp_server

## Run

Server:
sudo ./tftp_server

Client:
./tftp_client

## Client Commands

connect <IP> [port]
get <filename>
put <filename>
mode <octet|netascii>
help
bye / quit

**TFTP Port:** 69

