#pragma once

#include <cstdint>

/* Low-frequency ECU telemetry record — the slow-changing state logged to data_ext.bin
 * at ~10 Hz, separate from the 2 kHz EcuSystemState. The ECU has no thermocouples, so
 * for now this carries only the assembly timestamp; the shared event timestamps the
 * user intends (e.g. valve actuation begin/end) will land here, mirroring
 * FcuExtendedSystemState. Kept as a distinct per-board record so the two can diverge. */

struct EcuExtendedSystemState {
    uint32_t creation_timestamp_ms;     /**< Time this record was assembled. */
    uint32_t control_flags;             /**< Live control-flag bitmask (bit N = ControlFlag value N; see set_control_flag.hpp). Lets the GS read the recording config straight from telemetry. */
    uint8_t  last_refused_state_from;   /**< State the board was in when a SetState transition was last refused (State value). */
    uint8_t  last_refused_state_to;     /**< Requested state that was refused. Both 0 (Init) = none refused since boot. */
    uint8_t  reserved[2];               /**< Pad to 4 bytes; keep the record packed. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(EcuExtendedSystemState) == 3 * sizeof(uint32_t),
              "EcuExtendedSystemState has implicit padding — add explicit reserved bytes");
