#pragma once

#include <cstdint>

/** @brief Thermocouple status register (8-bit), the Status half of ThermocoupleInfo.
 *
 * The fault bits mirror the meaningful flags of the MAX31856 fault-status register
 * (SR, address 0x0F); data_valid is the driver's own "a clean read landed" flag. */
struct ThermocoupleStatus {
    uint8_t open_circuit : 1;  /**< SR.OPEN: thermocouple open / disconnected. */
    uint8_t over_under_v : 1;  /**< SR.OVUV: input voltage out of range (> Vdd or < Gnd). */
    uint8_t tc_out_range : 1;  /**< SR.TCRANGE: thermocouple temperature outside the type's range. */
    uint8_t cj_out_range : 1;  /**< SR.CJRANGE: cold-junction temperature outside range. */
    uint8_t data_valid   : 1;  /**< true if this record carries a fresh, fault-free read (not a fallback). */
    uint8_t reserved     : 3;  /**< Padding; room for the remaining SR threshold bits. */
};
static_assert(sizeof(ThermocoupleStatus) == 1, "ThermocoupleStatus must be exactly 1 byte (on the wire)");
