#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace emu {

class Memory {
public:
    explicit Memory(std::size_t size);

    std::uint8_t read8(std::uint32_t address) const;
    std::uint16_t read16(std::uint32_t address) const;
    std::uint32_t read32(std::uint32_t address) const;

    void write8(std::uint32_t address, std::uint8_t value);
    void write16(std::uint32_t address, std::uint16_t value);
    void write32(std::uint32_t address, std::uint32_t value);

    std::size_t size() const { return data_.size(); }

private:
    void check_range(std::uint32_t address, std::size_t width) const;

    std::vector<std::uint8_t> data_;
};

} // namespace emu
