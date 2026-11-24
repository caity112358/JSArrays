CC = gcc
UNAME := $(shell uname)

ifeq ($(UNAME),Darwin)
export PKG_CONFIG_PATH := $(shell brew --prefix libffi)/lib/pkgconfig:$(PKG_CONFIG_PATH)
endif

CFLAGS = -Wall -Werror -g $(shell pkg-config --cflags libffi criterion)
LDLIBS = $(shell pkg-config --libs libffi criterion)
SRC = jsArrays.c main.c
TARGET = jsArrayTest

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

asan: CC = clang
asan: CFLAGS += -fsanitize=address,undefined
asan: all

ifeq ($(UNAME),Darwin)
msan:
	@echo "MacOS has no msan"; exit 1
else
msan: CC = clang
msan: CFLAGS += -fsanitize=memory -fno-omit-frame-pointer
msan: all
endif

clean:
	rm -f $(TARGET)
	rm -rf $(TARGET).dSYM

.PHONY: all asan msan clean