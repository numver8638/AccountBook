SRCS = \
	Database.c \
	Input.c \
	LinkedList.c \
	Main.c \
	Menu.c

OUTPUT = theapp

$(OUTPUT): $(SRCS:%.c=%.o)
	$(CC) -o $@ $^

clean:
	$(RM) $(OUTPUT) $(SRCS:%.c=%.o)
