#include "../libgb/gb.hpp"

#include "sdl_io.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

auto main(int argc, char** argv) -> int {
  std::vector<std::string_view> args;
  for (int i = 1; i < argc; i++) {
    args.emplace_back(argv[i]);
  }

  // Extract + validate arguments
  std::optional<uint16_t> port;
  std::optional<std::string_view> rom;

  // Listen is named and implies gdb server mode
  if (auto listen_flag = std::ranges::find(args, std::string_view{"--listen"});
      listen_flag != args.end()) {
    const auto port_it = listen_flag + 1;
    port = std::stoi(std::string{*port_it});
    args.erase(listen_flag, port_it + 1);
  }

  // ROM is positional
  if (args.size() == 1) {
    rom = args[0];
    args.pop_back();
  }

  // Are there any remaining unparsed arguments?
  if (not args.empty()) {
    throw std::runtime_error(
        std::format("Argument error: unrecognized argument '{}'", args[0]));
  }

  // Run
  if (port.has_value()) {
    gb::run_gdb_server(*port, std::make_unique<SDLFrontend>(), rom);
  } else {
    if (not rom.has_value()) {
      throw std::runtime_error("Argument error: missing position argument ROM");
    }
    gb::run_standalone(std::make_unique<SDLFrontend>(), *rom);
  }
}
