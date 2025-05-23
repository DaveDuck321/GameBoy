#include "memory_map.hpp"

#include "cartridge.hpp"
#include "error_handling.hpp"
#include "io/io.hpp"

#include <cstdint>
#include <format>

using namespace gb;

MemoryMap::MemoryMap(Cartridge& cartridge, IO& io)
    : cartridge{&cartridge}, io{&io} {
  reset();
}

void MemoryMap::reset() {
  stack = {};
  workingRam = {};

  // Power up sequence (from http://bgb.bircd.org/pandocs.htm)
  write(0xFF05, 0x00_B);  // TIMA
  write(0xFF06, 0x00_B);  // TMA
  write(0xFF07, 0x00_B);  // TAC
  write(0xFF10, 0x80_B);  // NR10
  write(0xFF11, 0xBF_B);  // NR11
  write(0xFF12, 0xF3_B);  // NR12
  write(0xFF14, 0xBF_B);  // NR14
  write(0xFF16, 0x3F_B);  // NR21
  write(0xFF17, 0x00_B);  // NR22
  write(0xFF19, 0xBF_B);  // NR24
  write(0xFF1A, 0x7F_B);  // NR30
  write(0xFF1B, 0xFF_B);  // NR31
  write(0xFF1C, 0x9F_B);  // NR32
  write(0xFF1E, 0xBF_B);  // NR33
  write(0xFF20, 0xFF_B);  // NR41
  write(0xFF21, 0x00_B);  // NR42
  write(0xFF22, 0x00_B);  // NR43
  write(0xFF23, 0xBF_B);  // NR30
  write(0xFF24, 0x77_B);  // NR50
  write(0xFF25, 0xF3_B);  // NR51
  write(0xFF26, 0xF1_B);  // NR52
  write(0xFF40, 0x91_B);  // LCDC
  write(0xFF42, 0x00_B);  // SCY
  write(0xFF43, 0x00_B);  // SCX
  write(0xFF45, 0x00_B);  // LYC
  write(0xFF47, 0xFC_B);  // BGP
  write(0xFF48, 0xFF_B);  // OBP0
  write(0xFF49, 0xFF_B);  // OBP1
  write(0xFF4A, 0x00_B);  // WY
  write(0xFF4B, 0x00_B);  // WX
  write(0xFFFF, 0x00_B);  // IE

  write(0xFF0F, 0x00_B);  // Interrupts
}

void MemoryMap::DMA(uint8_t srcUpper) {
  if (srcUpper > 0xF1U) {
    throw_error([&] {
      return IllegalMemoryAddress(std::format(
          "Invalid upper address for DMA Transfer {:#06x}", srcUpper));
    });
    return;
  }

  io->startDMA();
  for (uint16_t offset = 0; offset < 0xA0U; offset++) {
    uint16_t srcAddr = static_cast<uint16_t>(srcUpper << 8U) | offset;
    uint16_t dstAddr = 0xFE00U | offset;
    write(dstAddr, read(srcAddr, /*is_dma=*/true), /*is_dma=*/true);
  }
}

auto MemoryMap::read(uint16_t addr, bool is_dma) const -> Byte {
  switch (addr) {
    case 0x0000 ... 0x7FFF:
      // Rom
      return cartridge->read(addr);
    case 0x8000 ... 0x9FFF:
      // Video Ram
      return Byte{io->videoRead(addr, is_dma)};
    case 0xA000 ... 0xBFFF:
      // External ram
      return cartridge->read(addr);
    case 0xC000 ... 0xDFFF:
      // Work ram 2
      return workingRam[addr - 0xC000];
    case 0xE000 ... 0xFDFF:
      // 0xC000 - 0xDDFF echo ram
      return workingRam[addr - 0xE000];
    case 0xFE00 ... 0xFE9F:
      // Sprite Attribute Table
      return Byte{io->videoRead(addr, is_dma)};
    case 0xFEA0 ... 0xFEFF:
      // Not Usable
      throw_error([&] {
        return IllegalMemoryAddress(
            std::format("Unusable memory address {:#06x}", addr));
      });
      return {};
    case 0xFF00 ... 0xFF7F:
      // IO ports
      return Byte{io->ioRead(addr)};
    case 0xFF80 ... 0xFFFE:
      // High ram
      return stack[addr - 0xFF80];
    case 0xFFFF:
      // Interrupts enabled Register
      return stack[0x7F];
    default:
      throw_error([&] {
        return IllegalMemoryAddress(
            std::format("Bad memory address {:#06x}", addr));
      });
      return {};
  }
}

auto MemoryMap::write(uint16_t addr, Byte value, bool is_dma) -> void {
  switch (addr) {
    case 0x0000 ... 0x7FFF:
      // Rom
      cartridge->write(addr, value);
      break;
    case 0x8000 ... 0x9FFF:
      // Video Ram
      io->videoWrite(addr, value.decay(), is_dma);
      break;
    case 0xA000 ... 0xBFFF:
      // External ram
      cartridge->write(addr, value);
      break;
    case 0xC000 ... 0xDFFF:
      // Work ram 2
      workingRam[addr - 0xC000] = value;
      break;
    case 0xE000 ... 0xFDFF:
      // 0xC000 - 0xDDFF echo ram
      workingRam[addr - 0xE000] = value;
      break;
    case 0xFE00 ... 0xFE9F:
      // Sprite Attribute Table
      io->videoWrite(addr, value.decay(), is_dma);
      break;
    case 0xFEA0 ... 0xFEFF:
      // Not Usable
      // Some games write anyway though...
      break;
    case 0xFF00 ... 0xFF45:
    case 0xFF47 ... 0xFF7F:
      // IO ports (except 0xFF46)
      io->ioWrite(addr, value.decay());
      break;
    case 0xFF46:
      // DMA -- DMA Transfer and start address (W)
      DMA(value.decay());
      break;
    case 0xFF80 ... 0xFFFE:
      // High ram
      stack[addr - 0xFF80] = value;
      break;
    case 0xFFFF:
      // Interrupts enabled Register
      stack[0x7F] = value;
      break;
    default:
      throw_error([&] {
        return IllegalMemoryAddress(
            std::format("Bad memory address {:#06x}", addr));
      });
      break;
  }
}
