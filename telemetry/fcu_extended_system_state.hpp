#pragma once

#include <cstdint>

#include "peripherals/thermocouple/thermocouple_info.hpp"   // ThermocoupleInfo, THERMOCOUPLE_COUNT
#include "peripherals/power_monitor/power_monitor_info.hpp"  // PowerMonitorInfo

/* Low-frequency FCU telemetry record — the slow-changing, bulky state downlinked
 * over Ethernet (and logged to data_slow.bin) at ~10 Hz, separate from the 2 kHz
 * SystemState. Keeping it apart avoids re-sending data that barely changes (the
 * thermocouples and the INA3221 are polled at ~10 Hz) thousands of times a second.
 *
 * FCU-only and flat for now: the 4 thermocouple channels and the INA3221 power
 * monitor. A shared base / ECU variant can be factored out later (as SystemStateBase
 * was) once the slow cross-board fields land — the user intends to add event
 * timestamps here (e.g. valve actuation begin/end). */

struct FcuExtendedSystemState {
    uint32_t         creation_timestamp_ms;                  /**< Time this record was assembled. */
    uint32_t         control_flags;                          /**< Live control-flag bitmask (bit N = ControlFlag value N; see set_control_flag.hpp). Lets the GS read the recording config straight from telemetry. */
    uint8_t          last_refused_state_from;                /**< State the board was in when a SetState transition was last refused (State value). */
    uint8_t          last_refused_state_to;                  /**< Requested state that was refused. Both 0 (Init) = none refused since boot. */
    uint8_t          reserved[2];                            /**< Pad to 4 bytes; keep the record packed. */
    ThermocoupleInfo thermocouple_info[THERMOCOUPLE_COUNT];  /**< 4 x MAX31856 (SPI6): per-channel temps + faults. */
    PowerMonitorInfo power_monitor;                          /**< INA3221 (I2C4): per-channel shunt/bus codes + faults. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(FcuExtendedSystemState) == 3 * sizeof(uint32_t)
                                              + THERMOCOUPLE_COUNT * sizeof(ThermocoupleInfo)
                                              + sizeof(PowerMonitorInfo),
              "FcuExtendedSystemState has implicit padding — add explicit reserved bytes");
