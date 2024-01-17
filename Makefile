CC = gcc
CC_FLAGS = -Wall -Wextra -Wpedantic -pthread

## Project file structure ##
SRC_DIR = src
OBJ_DIR = bin
INC_DIR = include

# Target executable name:
BIN = kernel

# Object files:
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/clock.o $(OBJ_DIR)/scheduler_dispatcher.o $(OBJ_DIR)/loader.o


## Compile ##
all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CC_FLAGS) -o $@ $^ -lm

# Compile main.c file to object files:
$(OBJ_DIR)/%.o : %.c
	$(CC) $(CC_FLAGS) -c $< -o $@

# Compile C source files to object files:
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@

# Clean objects in object directory.
.PHONY: clean
clean:
	$(RM) bin/* *.o $(BIN)