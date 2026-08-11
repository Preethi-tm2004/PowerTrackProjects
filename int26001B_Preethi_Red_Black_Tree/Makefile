CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = rbt

SRCS = main.c \
       create_node.c \
       insert.c \
       insert_fix.c \
       delete_node.c \
       delete_fix.c \
       delete_min.c \
       delete_max.c \
       search.c \
       min_max.c \
       left_rotate.c \
       right_rotate.c \
       transplant.c \
       black_height.c \
       validate_rbt.c \
       display.c \
       destroy_tree.c

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c rbt.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean $(TARGET)