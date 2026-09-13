#ifndef TFTP_FILE
#define TFTP_FILE

#include <stddef.h>
#include <sys/types.h>

int file_open_read(const char *filename);

ssize_t file_read(int fd, void *buffer, size_t size);

int file_open_write(const char *filename);

ssize_t file_write(int fd, const void *buffer, size_t size);

int file_close(int fd);

#endif
