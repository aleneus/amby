CC = gcc
CFLAGS = -Wall -Wextra -pthread
LIBS = -lasound -lm
BUILD_DIR = build
TARGET = $(BUILD_DIR)/amby

SRCS = $(wildcard src/*.c)

.PHONY: build todo clean

build:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

todo:
	@grep "TODO now" -rn src || true
