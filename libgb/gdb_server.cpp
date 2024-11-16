#ifndef __unix__

#include "io/frontend.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

using namespace gb;

auto gb::run_gdb_server(uint16_t,
                        std::unique_ptr<IOFrontend>,
                        std::optional<std::string_view>) -> void {
  throw std::runtime_error("GDB server mode is not supported.");
}

auto gb::load_from_elf(std::unique_ptr<gb::IOFrontend> frontend,
                       std::string_view elf_path) -> std::unique_ptr<gb::GB> {
  throw std::runtime_error("Toolchain integration is not supported.");
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
#include <utility>
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

auto gb::load_from_elf(std::unique_ptr<gb::IOFrontend> frontend,
                       std::string_view elf_path) -> std::unique_ptr<gb::GB> {
  // Use the toolchain to convert the ELF into a cartridge ROM
  std::string toolchain_prefix;
  if (const auto* toolchain_env = std::getenv("GB_TOOLCHAIN_BIN");
      toolchain_env != nullptr) {
    toolchain_prefix = std::string(toolchain_env) + "/";
  }

  auto* tmp_name = std::tmpnam(nullptr);
  run_command(std::format("{}llvm-objcopy -O binary {} {} --gap-fill 0",
                          toolchain_prefix, elf_path, tmp_name));

  // Load the emulator with the new rom file
  return std::make_unique<GB>(tmp_name, std::move(frontend));
}

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
        result.push_back(gb->readU8((uint16_t)(addr + offset)).decay_or(0xde));
      } catch (const IllegalMemoryAddress&) {
        // GDB misbehaves, just eat the error since it doesn't understand our
        // address space.
        result.push_back(0);
      }
    }
    return result;
  });

  server.add_read_register_value_callback(
      [&](size_t regno) -> std::optional<uint16_t> {
        if (regno > 5) {
          return std::nullopt;
        }
        return gb->getDebugRegisters().getU16(gb::Reg16(regno)).decay_or(0xde);
      });

  server.add_run_elf_callback([&](std::string_view elf) {
    std::cout << "Loading rom from elf: " << elf << std::endl;
    gb = gb::load_from_elf(std::move(frontend), elf);
  });
  server.add_is_attached_callback([&] { return gb != nullptr; });
  server.add_do_continue_callback([&](std::optional<size_t> addr) {
    if (addr.has_value()) {
      gb->getCurrentRegisters().pc = addr.value();
    }
    is_halted = false;
  });
  server.add_do_kill_callback([&]() {
    gb->reset();
    is_halted = true;
    return port;
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
        auto start_pc = gb->getCurrentRegisters().pc;
        while (gb->getCurrentRegisters().pc == start_pc) {
          gb->clock();
        }
      } catch (const BadOpcode&) {
        is_halted = true;
        server.notify_break(gdb::RemoteServer::BreakReason::SIGTRAP,
                            /*is_breakpoint=*/false);
        continue;
      } catch (const CorrectnessError& error) {
        std::cout << "Correctness error: breaking... " << error.what()
                  << std::endl;
        is_halted = true;
        server.notify_break(gdb::RemoteServer::BreakReason::SIGSEGV,
                            /*is_breakpoint=*/false);
        continue;
      }

      if (server.has_remote_interrupt_request()) {
        is_halted = true;
        server.notify_break(gdb::RemoteServer::BreakReason::SIGTRAP,
                            /*is_breakpoint=*/false);
        continue;
      }

      if (server.is_active_breakpoint(gb->getCurrentRegisters().pc)) {
        is_halted = true;
        server.notify_break(gdb::RemoteServer::BreakReason::SIGINT,
                            /*is_breakpoint=*/true);
      }
    }
  }
}

#endif
