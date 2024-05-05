#pragma once

#include "controller/controller.hpp"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gb {

class Cartridge {
  enum class Target : uint8_t {
    Classic = 0x00,
    Color = 0x80,
  };

  std::vector<uint8_t> rom;
  std::unique_ptr<Controller> controller;

  uint8_t controllerType = 0;  // Enum of controller technologies
  uint8_t romSize = 0;         // Enum of rom size
  uint8_t ramSize = 0;         // Enum of ram size

  std::string gameName;
  Target target = Target::Classic;

  explicit Cartridge(std::vector<uint8_t>&& rom);

 public:
  static auto loadFromRom(std::string_view name) -> Cartridge;

  void populateMetadata(const std::vector<uint8_t>& rom);

  [[nodiscard]] auto read(uint16_t addr) const -> uint8_t;
  auto write(uint16_t addr, uint8_t value) -> void;
};

// Controller type
auto make_mbc1(std::span<uint8_t> rom) -> std::unique_ptr<Controller>;
auto make_rom_only_controller(std::span<uint8_t> rom)
    -> std::unique_ptr<Controller>;

}  // namespace gb
