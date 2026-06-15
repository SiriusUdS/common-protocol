#pragma once

#include <cstdint>

#include "telemetry/extended_system_state_base.hpp"          // ExtendedSystemStateBase (shared prefix)
#include "peripherals/thermocouple/thermocouple_info.hpp"   // ThermocoupleInfo, THERMOCOUPLE_COUNT
#include "peripherals/power_monitor/power_monitor_info.hpp"  // PowerMonitorInfo
#include "devices/ematch/ematch_info.hpp"                    // EmatchInfo (presence + firing timeline)
#include "devices/solenoid/solenoid_info.hpp"                // SolenoidInfo (presence + actuation timeline)

/* Low-frequency FCU telemetry record — the slow-changing, bulky state downlinked
 * over Ethernet (and logged to data_slow.bin) at ~10 Hz, separate from the 2 kHz
 * SystemState. Keeping it apart avoids re-sending data that barely changes (the
 * thermocouples and the INA3221 are polled at ~10 Hz) thousands of times a second.
 *
 * The shared prefix (timestamp, control flags, refused-command diagnostics) is the
 * common ExtendedSystemStateBase; the FCU adds its board-specific peripherals (e-match,
 * solenoid, the 4 thermocouples, the INA3221) alongside it. */

struct FcuExtendedSystemState {
    ExtendedSystemStateBase base;                              /**< Shared prefix: timestamp + base/per-board control flags + refused-command diagnostics. */
    EmatchInfo         ematch_info;                            /**< E-match presence + firing-line state + last energise/deenergise ticks. */
    SolenoidInfo       solenoid_info;                          /**< Solenoid-valve presence + open/closed state + last open/close ticks. */
    ThermocoupleInfo   thermocouple_info[THERMOCOUPLE_COUNT];  /**< 4 x MAX31856 (SPI6): per-channel temps + faults. */
    PowerMonitorInfo   power_monitor;                          /**< INA3221 (I2C4): per-channel shunt/bus codes + faults. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(FcuExtendedSystemState) == sizeof(ExtendedSystemStateBase)
                                              + sizeof(EmatchInfo)
                                              + sizeof(SolenoidInfo)
                                              + THERMOCOUPLE_COUNT * sizeof(ThermocoupleInfo)
                                              + sizeof(PowerMonitorInfo),
              "FcuExtendedSystemState has implicit padding — add explicit reserved bytes");
