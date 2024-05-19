#ifndef __unix__

#include "io/frontend.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

using namespace gb;

auto run_gdb_server(uint16_t,
                    std::unique_ptr<IOFrontend>,
                    std::optional<std::string_view>) -> void {
  throw std::runtime_error("GDB server mode is not supported.");
}

#else

#include "error_handling.hpp"
#include "gb.hpp"
#include "gdb/remote_server.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <format>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace gb;

namespace {
auto run_command(std::string cmd) -> void {
  if (std::system(cmd.c_str()) != 0) {
    throw std::runtime_error(
        std::format("Command \"{}\" returned non-zero opcode", cmd));
  }
}
}  // namespace

void gb::run_gdb_server(uint16_t port,
                        std::unique_ptr<IOFrontend> frontend,
                        std::optional<std::string_view> rom_path) {
  std::unique_ptr<GB> gb;
  bool is_halted = true;

  if (rom_path.has_value()) {
    // ROM provided, load the emulator without waiting for the GDB client
    gb = std::make_unique<GB>(rom_path.value(), std::move(frontend));
  }

  // Setup the GDB server and callbacks
  gdb::RemoteServer server{{"af", "bc", "de", "hl", "sp", "pc"}};
  server.add_read_memory_callback([&](size_t addr, size_t size) {
    std::vector<uint8_t> result;
    for (size_t offset = 0; offset < size; offset++) {
      if (addr + offset > std::numeric_limits<uint16_t>::max()) {
        break;
      }

      try {
        result.push_back(gb->readU8((uint16_t)(addr + offset)));
      } catch (const IllegalMemoryRead&) {
        // GDB misbehaves, just eat the error since it doesn't understand our
        // address space.
        result.push_back(0);
      }
    }
    return result;
  });

  server.add_read_register_value_callback(
      [&](size_t regno) -> std::optional<uint16_t> {
        if (regno >= gb->getRegisters().r16_view.size()) {
          return std::nullopt;
        }
        return gb->getRegisters().r16_view[regno];
      });

  server.add_run_elf_callback([&](std::string_view elf) {
    std::cout << "Loading rom from elf: " << elf << std::endl;

    // Use the toolchain to convert the ELF into a cartridge ROM
    std::string toolchain_prefix;
    if (const auto* toolchain_env = std::getenv("GB_TOOLCHAIN_BIN");
        toolchain_env != nullptr) {
      toolchain_prefix = std::string(toolchain_env) + "/";
    }

    run_command(std::format("{}llvm-objcopy -O binary {} out.rom --gap-fill 0",
                            toolchain_prefix, elf));

    // Load the emulator
    gb = std::make_unique<GB>("out.rom", std::move(frontend));
  });
  server.add_is_attached_callback([&] { return gb != nullptr; });
  server.add_do_continue_callback([&](std::optional<size_t> addr) {
    if (addr.has_value()) {
      gb->getRegisters().r16.pc = addr.value();
    }
    is_halted = false;
  });

  // Start listening for connection
  server.wait_for_connection(port);

  // Wait for gdb to load a rom
  while (gb == nullptr) {
    server.process_next_request();
  }

  // Mainloop
  while (not gb->isSimulationFinished()) {
    if (is_halted) {
      server.process_next_request();
    } else {
      try {
        // PC isn't guaranteed to advance if waiting for an interrupt
        auto start_pc = gb->getRegisters().r16.pc;
        while (gb->getRegisters().r16.pc == start_pc) {
          gb->clock();
        }
      } catch (const BadOpcode&) {
        is_halted = true;
        server.notify_break(/*is_breakpoint=*/false);
        continue;
      }

      if (server.has_remote_interrupt_request()) {
        is_halted = true;
        server.notify_break(/*is_breakpoint=*/false);
        continue;
      }

      if (server.is_active_breakpoint(gb->getRegisters().r16.pc)) {
        is_halted = true;
        server.notify_break(/*is_breakpoint=*/true);
      }
    }
  }
}

#endif
