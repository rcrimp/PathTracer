CC = g++
SRCS = main.cpp
OBJS = $(SRCS:.c=.o)
TARGET = a.out
CFLAGS = 
LFLAGS = -lSDL2 -lGL -lGLEW
EXEC = pt.out

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LFLAGS) -o $(EXEC)
