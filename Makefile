TARGET=escape
build=automatic
arch=x86_64

SRC_EXT=c

SRCS=$(wildcard *.$(SRC_EXT))
OBJS=$(patsubst %.$(SRC_EXT),%.o,$(SRCS))

CFLAGS+=-I ../libnsp/icom/ -Wall -std=c89 -D_GNU_SOURCE

ifeq ($(build),debug)
	CFLAGS+=-g3
else
	CFLAGS+=-O2
endif

LD_DIR=/usr/local/lib64/
ifeq ($(arch), $(filter $(arch),arm arm32))
        CC=arm-linux-gnueabihf-g++
        OBJCOPY=arm-linux-gnueabihf-objcopy
        CFLAGS+=-mfloat-abi=hard -mfpu=neon
        LD_DIR=/usr/local/lib/
endif

ifeq ($(arch), $(filter $(arch),arm64 aarch64))
        CC=aarch64-linux-gnu-g++
        OBJCOPY=aarch64-linux-gnu-objcopy
        LD_DIR=/usr/local/lib/aarch64-linux-gnu/
endif

LDFLAGS+=-Wl,-rpath=$(LD_DIR) $(LD_DIR)nshost.so -lrt -lpthread -ldl

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.$(SRC_EXT)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY:clean all
