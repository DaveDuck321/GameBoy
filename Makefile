CC := g++
EXEC := a.out

DISPLAY := SDL
CFLAGS := -std=c++20 -O3 -std=c++20 -Wpedantic -Wall -Wextra 

INC_DIR := include
INC_FLAGS := $(addprefix -I, $(INC_DIR))
INC_DEPS := $(wildcard $(INC_DIR)/*.hpp) $(wildcard $(INC_DIR)/*/*.hpp)

ifeq ($(DISPLAY), SDL)
# For windows add '-lmingw32'
	DISP_Flags := -w -lSDL2main -lSDL2
endif

BUILD_DIR := build
SRC_DIR := src

SRCS := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/io/*.cpp) $(wildcard $(SRC_DIR)/io/$(DISPLAY)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(EXEC)

debug: CFLAGS += -DDEBUG -Wall -fsanitize=address,undefined -g -Og
debug: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $^ $(CFLAGS) $(DISP_Flags)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DEPS)
# Remove this and manually add directories for windows instead
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $< $(INC_FLAGS)
