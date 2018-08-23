SRC=$(wildcard ./app/*.c)
#SRC+=$(wildcard ./h264/*.c)
SRC+=$(wildcard ./lpnr/*.c)
SRC+=$(wildcard ./utils/*.c)
OBJ=$(SRC:%.c=%.o)
AIM=lp2hn

ARCH=arm
ifeq ($(ARCH) ,arm)
CC=arm-hisiv100-linux-gcc
LFLAGS=-lpthread -lm -lrt
CFLAGS:=-g -I. -DENABLE_TESTCODE -I./app -I./h264 -I./lpnr -I./utils -I./include
#CFLAGS+=-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -mno-unaligned-access -fno-aggressive-loop-optimizations
else
LFLAGS=-lpthread -lm -lrt
CFLAGS:=-g -I. -DENABLE_TESTCODE -I./app -I./h264 -I./lpnr -I./utils -I./include -DHENA_DEBUG
endif

all : $(AIM)
$(AIM) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJ) : %.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
			
.PHONY : clean
cc:
	rm -f $(OBJ)
	rm -f $(AIM)
