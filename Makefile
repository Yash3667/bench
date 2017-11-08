CC = gcc
FLAGS = -Wall -Wextra
BUILD_DIR = build
SRC_DIR = src
EXEC = $(BUILD_DIR)/bench
DEP = $(BUILD_DIR)/main.o $(BUILD_DIR)/cirq.o $(BUILD_DIR)/expdistrib.o

all: $(DEP)
	$(CC) -o $(EXEC) $(DEP) -lm -pthread

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(FLAGS) -g -o $(BUILD_DIR)/main.o -c $(SRC_DIR)/main.c
$(BUILD_DIR)/cirq.o: $(SRC_DIR)/cirq/cirq.c
	$(CC) $(FLAGS) -g -o $(BUILD_DIR)/cirq.o -c $(SRC_DIR)/cirq/cirq.c
$(BUILD_DIR)/expdistrib.o: $(SRC_DIR)/expdistrib/expdistrib.c
	$(CC) $(FLAGS) -g -o $(BUILD_DIR)/expdistrib.o -c $(SRC_DIR)/expdistrib/expdistrib.c

clean:
	rm -f $(BUILD_DIR)/*