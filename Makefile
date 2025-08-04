CC = cc
CFLAGS = -Wall -Wextra -I./src

SRC_DIR = src
OBJ_DIR = obj

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/system.c $(SRC_DIR)/auth.c $(SRC_DIR)/db.c
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

all: atm

atm: $(OBJECTS)
	$(CC) -o $@ $^ -lsqlite3

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/header.h
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) atm

run: atm
	./atm

.PHONY: all clean run
