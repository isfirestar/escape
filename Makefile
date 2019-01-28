TARGET=escape
build=automatic
arch=x86_64

SRC_EXT=cpp

SRCS=$(wildcard *.$(SRC_EXT)) $(wildcard ../libnsp/*.$(SRC_EXT))
OBJS=$(patsubst %.$(SRC_EXT),%.o,$(SRCS))

CFLAGS+=-I ../libnsp/ -Wall -std=c++11

ifeq ($(build),debug)
	CFLAGS+=-g
else
	CFLAGS+=-O2
endif

CC=g++
LDFLAGS+=./nshost.so -Wl,-rpath=./ -lrt -lpthread -ldl

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.$(SRC_EXT)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY:clean all
