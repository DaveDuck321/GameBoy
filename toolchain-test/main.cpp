#include "../libgb/gb.hpp"
#include "../libgb/io/headless.hpp"

#include <iostream>
#include <memory>
#include <string_view>

auto main(int argc, char** argv) -> int {
  std::vector<std::string_view> args;
  for (int i = 1; i < argc; i++) {
    args.emplace_back(argv[i]);
  }

  const auto elf_path = args[0];
  auto gb =
      gb::load_from_elf(std::make_unique<gb::Headless>(std::cout), elf_path);

  try {
    gb::run_standalone(*gb);
  } catch (const gb::Trap&) {
    std::cout << "done" << std::endl;
  }
}
