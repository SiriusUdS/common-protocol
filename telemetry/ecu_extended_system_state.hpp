#pragma once

#include <cstdint>

/* Low-frequency ECU telemetry record — the slow-changing state logged to data_ext.bin
 * at ~10 Hz, separate from the 2 kHz EcuSystemState. The ECU has no thermocouples, so
 * for now this carries only the assembly timestamp; the shared event timestamps the
 * user intends (e.g. valve actuation begin/end) will land here, mirroring
 * FcuExtendedSystemState. Kept as a distinct per-board record so the two can diverge. */

struct EcuExtendedSystemState {
    uint32_t creation_timestamp_ms;   /**< Time this record was assembled. */
    uint32_t control_flags;           /**< Live control-flag bitmask (bit N = ControlFlag value N; see set_control_flag.hpp). Lets the GS read the recording config straight from telemetry. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(EcuExtendedSystemState) == 2 * sizeof(uint32_t),
              "EcuExtendedSystemState has implicit padding — add explicit reserved bytes");
