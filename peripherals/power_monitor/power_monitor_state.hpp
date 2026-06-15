#pragma once

#include <cstdint>

/** @brief Current state of the power monitor (the State half of PowerMonitorInfo). */
enum class PowerMonitorState : uint8_t {
    Unknown = 0x00,  /**< Not yet configured / no read attempted. */
    Active  = 0x01,  /**< Converting — fresh shunt/bus reads are landing. */
    Faulted = 0x02,  /**< No valid reading (I2C/read error or the device not responding). */
};
static_assert(sizeof(PowerMonitorState) == 1, "PowerMonitorState must be exactly 1 byte (on the wire)");
