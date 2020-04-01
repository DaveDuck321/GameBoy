CC := g++
EXEC := a.exe
CFLAGS := -g -O0 -DDEBUG

INC_DIR := include
INC_FLAGS := $(addprefix -I, $(INC_DIR))
INC_DEPS := $(wildcard $(INC_DIR)/*.hpp)

BUILD_DIR := build
SRC_DIR := src

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $^

$(BUILD_DIR)/cartridge.o: $(SRC_DIR)/cartridge.cpp $(INC_DIR)/cartridge.hpp
	$(CC) $(CFLAGS) -o $@ -c $< $(INC_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DEPS)
	$(CC) $(CFLAGS) -o $@ -c $< $(INC_FLAGS)
