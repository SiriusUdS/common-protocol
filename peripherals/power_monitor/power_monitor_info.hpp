#pragma once

#include <cstdint>

#include "peripherals/power_monitor/power_monitor_state.hpp"
#include "peripherals/power_monitor/power_monitor_status.hpp"

/* Telemetry unit for the power monitor (a 3-channel INA3221): its state, status and
 * the latest per-channel shunt + bus voltage register codes, owned and kept current
 * by the driver. As with AdcInfo's raw counts and ThermocoupleInfo's temperature
 * codes, the measurements are stored as the device's own sign-extended register
 * values; the ground station applies the LSB scale, so no rounding happens on board. */

/** @brief Channels the INA3221 monitors (3-channel part). */
inline constexpr unsigned POWER_MONITOR_CHANNEL_COUNT = 3;

/** @brief One channel's two measurements (shunt drop -> current, bus -> rail voltage). */
struct PowerMonitorChannel {
    int16_t shunt_code;  /**< Shunt-voltage register, sign-extended; LSB = 40 µV (current = code·40µV / Rshunt). */
    int16_t bus_code;    /**< Bus-voltage register, sign-extended; LSB = 8 mV. */
};
static_assert(sizeof(PowerMonitorChannel) == 2 * sizeof(int16_t),
              "PowerMonitorChannel has implicit padding — keep it two packed int16 codes");

/** @brief The power monitor's reported state + status + per-channel measurements. */
struct PowerMonitorInfo {
    PowerMonitorState   state;
    PowerMonitorStatus  status;
    uint8_t             reserved[2];  /**< Aligns the channels to 2 bytes; keeps the record packed. */
    PowerMonitorChannel channels[POWER_MONITOR_CHANNEL_COUNT];
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(PowerMonitorInfo) == 4 + POWER_MONITOR_CHANNEL_COUNT * sizeof(PowerMonitorChannel),
              "PowerMonitorInfo has implicit padding — keep state/status/reserved at 4 bytes then the channels");
