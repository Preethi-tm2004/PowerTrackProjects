CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = text_editor

SRC = main.c text_editor.c undo_redo.c 

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
	