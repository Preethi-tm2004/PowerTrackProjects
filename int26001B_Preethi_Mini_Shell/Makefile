CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = minishell

SRCS = main.c \
       scan.c \
       check_cmd_type.c \
       execute_external_commands.c \
       execute_internal_commands.c \
       n_pipe.c \
       job_list.c

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c main.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean $(TARGET)

.PHONY: clean fclean re
