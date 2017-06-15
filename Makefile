TARGET=escape

CC=g++
SRCS=$(wildcard *.cpp) $(wildcard ../libnsp/*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

CPPFLAGS+=-I ../libnsp/ -Wall -std=c++11
LDFLAGS=/usr/local/lib64/nshost.so -Wl,-rpath=/usr/local/lib64 -lrt -lpthread -ldl

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY:clean all
