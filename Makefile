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

ifeq ($(arch),arm)
	CC=arm-linux-gnueabihf-g++
	CFLAGS+=-mfloat-abi=hard -mfpu=neon
	LDFLAGS+=/usr/local/lib/nshost.so -Wl,-rpath=/usr/local/lib/
else
	ifeq ($(arch), i686)
		CC=g++
		CFLAGS+=-m32
		LDFLAGS+=-m32
		LDFLAGS+=/usr/local/lib/nshost.so -Wl,-rpath=/usr/local/lib/
	else
		CC=g++
		LDFLAGS+=/usr/local/lib64/nshost.so -Wl,-rpath=/usr/local/lib64/
	endif
endif

LDFLAGS+=-lrt -lpthread -ldl

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.$(SRC_EXT)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY:clean all
