PROGRAM = server
OBJS = daemon_init.o error.o readline.o str_echo.o wrapsocket.o server.o
SRCS = $(OBJS:%.o=%.c)
CFLAGS = -g -Wall -O2
LDFLAGS = 
CC = gcc


$(PROGRAM):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LDLIBS)
