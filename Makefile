TARGET=escape
build=automatic
arch=x86_64

SRCS=$(wildcard *.cpp) $(wildcard ../libnsp/*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

CPPFLAGS+=-I ../libnsp/ -Wall -std=c++11
LDFLAGS=/usr/local/lib64/nshost.so -Wl,-rpath=/usr/local/lib64 -lrt -lpthread -ldl

ifeq ($(build),debug)
	CFLAGS+=-g
else
	CFLAGS+=-O2
endif

ifeq ($(arch),arm)
	CC=arm-linux-gnueabihf-g++
else
	CC=g++
endif

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY:clean all
