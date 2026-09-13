#include "file.h"

#include <fcntl.h>
#include <unistd.h>

int file_open_read(const char *filename)
{
    return open(filename, O_RDONLY);
}

ssize_t file_read(int fd, void *buffer, size_t size)
{
    return read(fd, buffer, size);
}

int file_open_write(const char *filename)
{
    return open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
}

ssize_t file_write(int fd, const void *buffer, size_t size)
{
    return write(fd, buffer, size);
}

int file_close(int fd)
{
    return close(fd);
}