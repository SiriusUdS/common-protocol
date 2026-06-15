#pragma once

#include <cstdint>

/** @brief Power-monitor status register (8-bit), the Status half of PowerMonitorInfo.
 *
 * The INA3221 has no single fault-status byte like the MAX31856's SR; these are the
 * driver's own flags. data_valid says a fresh, complete round of channel reads landed;
 * read_error says the last I2C transfer NACKed / errored / timed out. */
struct PowerMonitorStatus {
    uint8_t data_valid : 1;  /**< true if this record carries a fresh, complete read (not a fallback). */
    uint8_t read_error : 1;  /**< last I2C transfer failed (NACK / bus error / timeout). */
    uint8_t reserved   : 6;  /**< Padding; room for INA3221 alert/limit flags later. */
};
static_assert(sizeof(PowerMonitorStatus) == 1, "PowerMonitorStatus must be exactly 1 byte (on the wire)");
