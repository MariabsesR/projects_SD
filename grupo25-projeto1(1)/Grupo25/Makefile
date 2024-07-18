# Directory structure
BIN_DIR = binary
INC_DIR = include
SRC_DIR = source
OBJ_DIR = object

# Compiler and flags
CC = gcc
CFLAGS = -Wall -MMD -MP -I$(INC_DIR)

# Executable targets
EXECS = $(BIN_DIR)/test_data $(BIN_DIR)/test_entry $(BIN_DIR)/test_list $(BIN_DIR)/test_table $(BIN_DIR)/test_serialization

# Default target (all executables)
make: $(EXECS)

# Compile each test program
$(BIN_DIR)/test_data: $(OBJ_DIR)/test_data.o $(OBJ_DIR)/data.o
	$(CC) $^ -o $@

$(BIN_DIR)/test_entry: $(OBJ_DIR)/test_entry.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/data.o
	$(CC) $^ -o $@

$(BIN_DIR)/test_list: $(OBJ_DIR)/test_list.o $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/data.o
	$(CC) $^ -o $@

$(BIN_DIR)/test_table: $(OBJ_DIR)/test_table.o $(OBJ_DIR)/table.o $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/data.o
	$(CC) $^ -o $@

$(BIN_DIR)/test_serialization: $(OBJ_DIR)/test_serialization.o $(OBJ_DIR)/serialization.o $(OBJ_DIR)/table.o $(OBJ_DIR)/list.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/data.o
	$(CC) $^ -o $@

# Build each object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


-include $(wildcard $(OBJ_DIR)/*.d)

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*


