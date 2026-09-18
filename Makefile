CC = gcc
CFLAGS = -Wall -g -Iinclude
BUILD_DIR = build
SRC_DIR = src

CLIENT_BIN = $(BUILD_DIR)/client
SERVER_BIN = $(BUILD_DIR)/server

.PHONY: all clean

all: $(CLIENT_BIN) $(SERVER_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(CLIENT_BIN): $(SRC_DIR)/udpClient.c $(SRC_DIR)/utils.c $(SRC_DIR)/sha256.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(SERVER_BIN): $(SRC_DIR)/udpServer.c $(SRC_DIR)/utils.c $(SRC_DIR)/sha256.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(CLIENT_BIN) $(SERVER_BIN)