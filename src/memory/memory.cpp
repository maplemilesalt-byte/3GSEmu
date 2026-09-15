#include "memory/memory.h"

#include <stdexcept>

namespace emu {

Memory::Memory(std::size_t size) : data_(size, 0) {}

void Memory::check_range(std::uint32_t address, std::size_t width) const {
    const auto start = static_cast<std::size_t>(address);
    if (start > data_.size() || width > data_.size() - start) {
        throw std::out_of_range("emulated memory access out of range");
    }
}

std::uint8_t Memory::read8(std::uint32_t address) const {
    check_range(address, 1);
    return data_[address];
}

std::uint32_t Memory::read32(std::uint32_t address) const {
    check_range(address, 4);

    return static_cast<std::uint32_t>(data_[address]) |
           (static_cast<std::uint32_t>(data_[address + 1]) << 8) |
           (static_cast<std::uint32_t>(data_[address + 2]) << 16) |
           (static_cast<std::uint32_t>(data_[address + 3]) << 24);
}

void Memory::write8(std::uint32_t address, std::uint8_t value) {
    check_range(address, 1);
    data_[address] = value;
}

void Memory::write32(std::uint32_t address, std::uint32_t value) {
    check_range(address, 4);

    data_[address] = static_cast<std::uint8_t>(value);
    data_[address + 1] = static_cast<std::uint8_t>(value >> 8);
    data_[address + 2] = static_cast<std::uint8_t>(value >> 16);
    data_[address + 3] = static_cast<std::uint8_t>(value >> 24);
}

} // namespace emu
