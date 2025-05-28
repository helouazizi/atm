CC = cc
CFLAGS = -Wall -Wextra -I./src

SRC_DIR = src
OBJ_DIR = obj

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/system.c $(SRC_DIR)/auth.c
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Default target
all: atm

# Link the final executable
atm: $(OBJECTS)
	$(CC) -o $@ $^

# Compile .c to .o in obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/header.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) atm

# Run the program (make run)
run: atm
	./atm

