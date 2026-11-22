CC = gcc
CFLAGS = -Wall -Werror -g
LDLIBS = -lffi -lcriterion
SRC = jsArrays.c main.c
TARGET = jsArrayTest

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

asan: CC = clang
asan: CFLAGS += -fsanitize=address,undefined
asan: all

msan: CC = clang
msan: CFLAGS += -fsanitize=memory -fno-omit-frame-pointer
msan: all

clean:
	rm -f $(TARGET)

.PHONY: all asan msan clean
