TARGET=escape
build=automatic
arch=x86_64

SRC_EXT=cpp

SRCS=$(wildcard *.$(SRC_EXT)) $(wildcard ../libnsp/*.$(SRC_EXT))
OBJS=$(patsubst %.$(SRC_EXT),%.o,$(SRCS))

CFLAGS+=-I ../libnsp/ -Wall -std=c++11
LDFLAGS=-lrt -lpthread -ldl

ifeq ($(build),debug)
	CFLAGS+=-g
else
	CFLAGS+=-O2
endif

ifeq ($(arch),arm)
	CC=arm-linux-gnueabihf-g++
	CFLAGS+=-mfloat-abi=hard -mfpu=neon
	LDFLAGS+=/usr/local/lib/nshost.so -Wl,-rpath=/usr/local/lib/
else
	CC=g++
	LDFLAGS+=/usr/local/lib64/nshost.so -Wl,-rpath=/usr/local/lib64/
endif

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.$(SRC_EXT)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY:clean all
