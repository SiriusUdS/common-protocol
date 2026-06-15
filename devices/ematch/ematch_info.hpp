#pragma once

#include <cstdint>

/* Telemetry unit for the FCU e-match (igniter), owned and kept current by the e-match:
 * its status (presence + firing-line state) and the ticks of the last energise/deenergise
 * edges. It rides the low-rate FcuExtendedSystemState so the ground station reads igniter
 * presence, arming, and the exact firing timeline straight from telemetry. Same shape as
 * the other *Info records (state/status + data), minus a state enum — the e-match has no
 * device state machine of its own. */

/** @brief E-match status (8-bit): presence + firing-line state. */
struct EmatchStatus {
    uint8_t detected  : 1;  /**< An e-match is plugged in (EMATCH_DET active / continuity LED lit). */
    uint8_t energised : 1;  /**< Firing line driven high (held only during the Ignite state). */
    uint8_t reserved  : 6;  /**< Padding; room for more e-match flags later. */
};
static_assert(sizeof(EmatchStatus) == 1, "EmatchStatus must be exactly 1 byte (on the wire)");

/** @brief The e-match's reported status + the firing timeline (last energise/deenergise ticks). */
struct EmatchInfo {
    EmatchStatus status;               /**< Presence (detected) + firing-line state (energised). */
    uint8_t      reserved[3];          /**< Aligns the timestamps to 4 bytes; keeps the record packed. */
    uint32_t     last_energised_ms;    /**< Tick of the last energise (0 = never energised since boot). */
    uint32_t     last_deenergised_ms;  /**< Tick of the last deenergise (0 = never deenergised since boot). */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(EmatchInfo) == 4 + 2 * sizeof(uint32_t),
              "EmatchInfo has implicit padding — keep status/reserved at 4 bytes then the two timestamps");
