#pragma once

#include <cstdint>

#include "peripherals/power_monitor/power_monitor_info.hpp"  // PowerMonitorInfo

/* Low-frequency ECU telemetry record — the slow-changing state logged to data_ext.bin
 * at ~10 Hz, separate from the 2 kHz EcuSystemState. The ECU has no thermocouples but
 * carries the INA3221 power monitor (like the FCU). The shared event timestamps the
 * user intends (e.g. valve actuation begin/end) will land here too. Kept as a distinct
 * per-board record so the two can diverge.
 *
 * NOTE: on the current ECU PCB the I2C lines are swapped, so the driver runs stubbed and
 * power_monitor reads Unknown / data_valid 0 until the board is fixed (see ina3221 Config). */

struct EcuExtendedSystemState {
    uint32_t         creation_timestamp_ms;   /**< Time this record was assembled. */
    uint32_t         control_flags;           /**< Live control-flag bitmask (bit N = ControlFlag value N; see set_control_flag.hpp). Lets the GS read the recording config straight from telemetry. */
    uint8_t          last_refused_state_from; /**< State the board was in when a SetState transition was last refused (State value). */
    uint8_t          last_refused_state_to;   /**< Requested state that was refused. Both 0 (Init) = none refused since boot. */
    uint8_t          reserved[2];             /**< Pad to 4 bytes; keep the record packed. */
    PowerMonitorInfo power_monitor;           /**< INA3221 (I2C4): per-channel shunt/bus codes + faults. Stubbed on the current PCB. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(EcuExtendedSystemState) == 3 * sizeof(uint32_t) + sizeof(PowerMonitorInfo),
              "EcuExtendedSystemState has implicit padding — add explicit reserved bytes");
