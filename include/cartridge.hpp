#ifndef cartridge_hpp
#define cartridge_hpp
#include <type_traits>

#include <vector>
#include <string>

#include <iostream>

class Cartridge
{
    private:

    public:
    std::string gameName; //Self reported name
    uint8_t gbType; //0x80 = Color GB
    uint8_t cartridgeType; //Enum of cartridge technologies
    uint8_t romSize, ramSize; //Enum of specification

    std::vector<uint8_t> rom;

    static Cartridge loadRom(const std::string& name);

    // Can move but not copy
    Cartridge(const Cartridge &) = delete;
    Cartridge(Cartridge&&) = default;

    template <class _Iter, class = std::enable_if_t<
        std::is_same<
            typename std::iterator_traits<_Iter>::value_type,
            char
        >::value
    >>
    Cartridge(_Iter rom_start, _Iter rom_end)
    {
        rom.assign(rom_start, rom_end);
        populateMetadata();
    }
    void populateMetadata();

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);
};

#endif