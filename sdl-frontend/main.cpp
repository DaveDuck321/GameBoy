#include "libgb/cartridge.hpp"
#include "libgb/gb.hpp"
#include "libgb/io/headless.hpp"

#include "sdl_io.hpp"

#include <chrono>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

void runGame(const char* rom) {
  /*
  Creates a graphical output and emulates the chosen ROM at 60Hz.
  Input, output and timing is handled by the SDL_IO class.
  */
  gb::GB gb(rom, std::make_unique<SDLFrontend>());
  while (not gb.isSimulationFinished()) {
    gb.clock();
  }
}

auto main(int argc, char** argv) -> int {
  if (argc != 2) {
    std::runtime_error("Expected 1 argument");
  }
  runGame(argv[1]);
  return EXIT_SUCCESS;
}
