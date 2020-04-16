CC := g++
EXEC := a.out
CFLAGS := -g -O0 -DDEBUG -Wall

INC_DIR := include
INC_FLAGS := $(addprefix -I, $(INC_DIR))
INC_DEPS := $(wildcard $(INC_DIR)/*.hpp)

SDL_FLAGS := -lSDL2main -lSDL2

BUILD_DIR := build
SRC_DIR := src

SRCS := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/SDL/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $^ $(CFLAGS) $(SDL_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DEPS)
	$(CC) $(CFLAGS) -o $@ -c $< $(INC_FLAGS)
