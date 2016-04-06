CC = g++
SRCS = main.cpp math.cpp
OBJS = $(SRCS:.c=.o)
TARGET = pt.out
CFLAGS = -I Eigen/ -O3 -std=c++11 -g
LFLAGS = -lSDL2 -lGL -lGLEW

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LFLAGS) -o $(TARGET)
