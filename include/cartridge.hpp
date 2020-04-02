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
    }

    uint8_t read(uint16_t location);
};

#endif