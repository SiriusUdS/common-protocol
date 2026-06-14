#pragma once

#include <cstdint>

#include "peripherals/thermocouple/thermocouple_state.hpp"
#include "peripherals/thermocouple/thermocouple_status.hpp"

/* Telemetry unit for one thermocouple channel (a MAX31856): its state, status and
 * the latest linearized thermocouple + cold-junction temperatures, owned and kept
 * current by the driver. As with AdcInfo's raw counts, the temperatures are stored
 * as the device's own sign-extended register codes (a fixed-point °C); the ground
 * station applies the LSB scale, so no rounding happens on the board. */

/** @brief Number of thermocouple channels on the FCU (4 x MAX31856 on SPI6). */
inline constexpr unsigned THERMOCOUPLE_COUNT = 4;

/** @brief One thermocouple's reported state + status + temperatures. */
struct ThermocoupleInfo {
    ThermocoupleState  state;
    ThermocoupleStatus status;
    uint8_t            reserved[2];        /**< Aligns the temperatures to 4 bytes; keeps the record packed. */
    int32_t            thermocouple_code;  /**< Linearized thermocouple temp, sign-extended; LSB = 2^-7 °C (0.0078125). */
    int32_t            cold_junction_code; /**< Cold-junction temp, sign-extended; LSB = 2^-6 °C (0.015625). */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(ThermocoupleInfo) == 4 + 2 * sizeof(int32_t),
              "ThermocoupleInfo has implicit padding — keep state/status/reserved at 4 bytes then the two codes");
