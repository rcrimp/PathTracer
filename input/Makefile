CC = g++
SRCS = main.cpp
OBJS = $(SRCS:.c=.o)
TARGET = a.out
CFLAGS = -I ../Eigen/
LFLAGS = # -lSDL2 -lGL -lGLEW

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LFLAGS) -o $(TARGET)
