#pragma once

#include <cstdint>

#include "telemetry/extended_system_state_base.hpp"          // ExtendedSystemStateBase (shared prefix)
#include "peripherals/power_monitor/power_monitor_info.hpp"  // PowerMonitorInfo

/* Low-frequency ECU telemetry record — the slow-changing state logged to data_ext.bin
 * at ~10 Hz and downlinked to the FCU (which relays it to the GS), separate from the 2 kHz
 * EcuSystemState. The ECU has no thermocouples and no e-match / solenoid (those are FCU
 * board-specific), so beyond the shared prefix it carries only the INA3221 power monitor.
 *
 * The shared prefix (timestamp, base/per-board control flags, refused-command diagnostics)
 * is the common ExtendedSystemStateBase — identical to the FCU's. The ECU has no per-board
 * flags, so base.control_flags_board stays 0. */

struct EcuExtendedSystemState {
    ExtendedSystemStateBase base;           /**< Shared prefix: timestamp + base/per-board control flags + refused-command diagnostics. */
    PowerMonitorInfo        power_monitor;  /**< INA3221 (I2C4): per-channel shunt/bus codes + faults. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(EcuExtendedSystemState) == sizeof(ExtendedSystemStateBase)
                                              + sizeof(PowerMonitorInfo),
              "EcuExtendedSystemState has implicit padding — add explicit reserved bytes");
