#ifndef cartridge_hpp
#define cartridge_hpp

#include "controller.hpp"

#include <memory>

#include <vector>
#include <string>

#include <iostream>

class Cartridge
{
    private:
    std::unique_ptr<Controller> controller;

    uint8_t cartridgeType; //Enum of cartridge technologies
    uint8_t romSize, ramSize; //Enum of specification

    public:
    std::string gameName; //Self reported name
    uint8_t gbType; //0x80 = Color GB

    static Cartridge loadRom(const std::string& name);

    // Can move but not copy
    Cartridge(const Cartridge &) = delete;
    Cartridge(Cartridge&&) = default;

    Cartridge(std::vector<uint8_t>&& rom);
    void populateMetadata(const std::vector<uint8_t>& rom);

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);
};

#endif