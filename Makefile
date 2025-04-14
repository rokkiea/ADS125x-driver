CC = gcc
CFLAGS = -Wall -g

SRCS = src/ads1256.c src/libads1256/libads1256.c
INC_DIRS = src src/libads1256
GPIOD_INCLUDE_DIR = /usr/include
GPIOD_LIB_NAME = gpiod
GPIOD_LIB_DIR = /usr/lib/aarch64-linux-gnu
CFLAGS += -I$(GPIOD_INCLUDE_DIR)
CFLAGS += $(addprefix -I,$(INC_DIRS))
OBJS = src/ads1256.o src/libads1256/libads1256.o
LDFLAGS += -L$(GPIOD_LIB_DIR) -l$(GPIOD_LIB_NAME)

PROJ_ROOT = $(abspath ../..)
TMP_PATH = $(abspath .)/tmp
PWD_PATH = $(abspath .)

# INCLUDE_PATH += 

# TARGET := ${PWD_PATH}/target/ads1256
TARGET = ads1256

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

src/ads1256.o: src/ads1256.c src/ads1256.h src/libads1256/libads1256.h src/libads1256/libads1256reg.h
	$(CC) $(CFLAGS) -c src/ads1256.c -o src/ads1256.o
src/libads1256/libads1256.o: src/libads1256/libads1256.c src/libads1256/libads1256.h src/libads1256/libads1256reg.h
	$(CC) $(CFLAGS) -c src/libads1256/libads1256.c -o src/libads1256/libads1256.o

clean:
	rm -f $(OBJS) $(TARGET)