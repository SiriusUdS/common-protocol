#pragma once

#include <cstdint>

#include "peripherals/thermocouple/thermocouple_info.hpp"   // ThermocoupleInfo, THERMOCOUPLE_COUNT

/* Low-frequency FCU telemetry record — the slow-changing, bulky state downlinked
 * over Ethernet (and logged to data_slow.bin) at ~10 Hz, separate from the 2 kHz
 * SystemState. Keeping it apart avoids re-sending data that barely changes (the
 * thermocouples are polled at ~10 Hz) thousands of times a second.
 *
 * FCU-only and flat for now: it carries just the 4 thermocouple channels. A shared
 * base / ECU variant can be factored out later (as SystemStateBase was) once the
 * slow cross-board fields land — the user intends to add event timestamps here
 * (e.g. valve actuation begin/end). */

struct FcuExtendedSystemState {
    uint32_t         creation_timestamp_ms;                  /**< Time this record was assembled. */
    uint32_t         control_flags;                          /**< Live control-flag bitmask (bit N = ControlFlag value N; see set_control_flag.hpp). Lets the GS read the recording config straight from telemetry. */
    ThermocoupleInfo thermocouple_info[THERMOCOUPLE_COUNT];  /**< 4 x MAX31856 (SPI6): per-channel temps + faults. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(FcuExtendedSystemState) == 2 * sizeof(uint32_t)
                                              + THERMOCOUPLE_COUNT * sizeof(ThermocoupleInfo),
              "FcuExtendedSystemState has implicit padding — add explicit reserved bytes");
