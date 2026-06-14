#pragma once

#include <cstdint>

/** @brief Current state of a thermocouple channel (the State half of ThermocoupleInfo). */
enum class ThermocoupleState : uint8_t {
    Unknown = 0x00,  /**< Not yet configured / no read attempted. */
    Active  = 0x01,  /**< Converting — fresh temperature reads are landing. */
    Faulted = 0x02,  /**< No valid reading (SPI/read error or a hard sensor fault). */
};
static_assert(sizeof(ThermocoupleState) == 1, "ThermocoupleState must be exactly 1 byte (on the wire)");
