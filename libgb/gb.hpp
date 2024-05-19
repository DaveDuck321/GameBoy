#pragma once

#include "cartridge.hpp"
#include "cpu/cpu.hpp"
#include "io/io.hpp"
#include "memory_map.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

// http://bgb.bircd.org/pandocs.htm
// http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf <-- useful but dont trust
// https://github.com/gbdev/awesome-gbdev#testing
// https://rednex.github.io/rgbds/gbz80.7.html
// https://pastraiser.com/cpu/gameboy/gameboy_opcodes.html
// https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf <--
// Amazing, wish I found this sooner

namespace gb {

class GB {
 public:
  Cartridge cartridge;
  IO io;
  MemoryMap memory_map;
  CPU cpu;

  GB(std::string_view rom_file, std::unique_ptr<IOFrontend> io_frontend);

  // Consume 0 CPU cycles
  [[nodiscard]] auto readU8(uint16_t addr) const -> uint8_t;
  [[nodiscard]] auto readU16(uint16_t addr) const -> uint16_t;

  [[nodiscard]] auto getRegisters() -> CPURegisters&;
  auto isSimulationFinished() -> bool;
  auto clock() -> void;

  // Debug
  auto insertInterruptOnNextCycle(uint8_t id) -> void;
};

auto run_standalone(std::unique_ptr<gb::IOFrontend>, std::string_view rom_path)
    -> void;

auto run_gdb_server(uint16_t port,
                    std::unique_ptr<gb::IOFrontend>,
                    std::optional<std::string_view> rom_path) -> void;

}  // namespace gb
