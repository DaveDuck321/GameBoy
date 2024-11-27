CXX := g++
AR := ar
EXEC := a.out

DISPLAY := SDL
CXX_FLAGS := -std=gnu++23 -O3 -Wall -Wextra -g

BUILD_DIR := build

LIBGB = $(BUILD_DIR)/libgb.a
LIBGB_SOURCES = $(wildcard libgb/*.cpp) $(wildcard libgb/**/*.cpp)
LIBGB_OBJ = $(LIBGB_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
LIBGB_DEP = $(LIBGB_OBJ:%.o=%.d)

HEADLESS_TESTS = tests.out
HEADLESS_TESTS_SOURCES = tests/main.cpp
HEADLESS_TESTS_OBJ = $(HEADLESS_TESTS_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
HEADLESS_TESTS_DEP = $(HEADLESS_TESTS_OBJ:%.o=%.d)

TOOLCHAIN_TESTS = emulate.out
TOOLCHAIN_TESTS_SOURCES = toolchain-test/main.cpp
TOOLCHAIN_TESTS_OBJ = $(TOOLCHAIN_TESTS_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TOOLCHAIN_TESTS_DEP = $(TOOLCHAIN_TESTS_OBJ:%.o=%.d)

SDL_DISPLAY = gb.out
SDL_DISPLAY_SOURCES = $(wildcard sdl-frontend/*.cpp)
SDL_DISPLAY_OBJ = $(SDL_DISPLAY_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
SDL_DISPLAY_DEP = $(SDL_DISPLAY_OBJ:%.o=%.d)
SDL_LD_FLAGS = -lSDL2

.PHONY : all

all: $(HEADLESS_TESTS)
$(HEADLESS_TESTS): $(HEADLESS_TESTS_OBJ) $(LIBGB)
	$(CXX) $(CXX_FLAGS) $^ -o $@

all: $(TOOLCHAIN_TESTS)
$(TOOLCHAIN_TESTS): $(TOOLCHAIN_TESTS_OBJ) $(LIBGB)
	$(CXX) $(CXX_FLAGS) $^ -o $@

all: $(SDL_DISPLAY)
$(SDL_DISPLAY): $(SDL_DISPLAY_OBJ) $(LIBGB)
	$(CXX) $(CXX_FLAGS) $(SDL_LD_FLAGS) $^ -o $@

$(LIBGB): $(LIBGB_OBJ)
	$(AR) -crs $(LIBGB) $^

-include $(LIBGB_DEP)
-include $(HEADLESS_TESTS_DEP)
-include $(TOOLCHAIN_TESTS_DEP)
-include $(SDL_DISPLAY_DEP)

$(BUILD_DIR)/libgb/%.o : libgb/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -Ilibgb -c $< -o $@

$(BUILD_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -I. -c $< -o $@

.PHONY : tests
tests: $(HEADLESS_TESTS)

.PHONY : clean
clean:
	-rm $(LIBGB_OBJ) $(LIBGB_DEP) $(LIBGB)\
		$(HEADLESS_TESTS_DEP) $(HEADLESS_TESTS_OBJ) $(HEADLESS_TESTS)	\
		$(SDL_DISPLAY_DEP) $(SDL_DISPLAY_OBJ) $(SDL_DISPLAY) \
		$(TOOLCHAIN_TESTS_OBJ) $(TOOLCHAIN_TESTS_DEP) $(TOOLCHAIN_TESTS) 2> /dev/null
