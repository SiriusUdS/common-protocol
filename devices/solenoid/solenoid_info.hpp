#pragma once

#include <cstdint>

/* Telemetry unit for the FCU solenoid valve, owned and kept current by the solenoid: its
 * status (presence + open/closed state) and the ticks of the last open/close edges. It rides
 * the low-rate FcuExtendedSystemState so the ground station reads solenoid presence, whether
 * it is driven open, and the exact actuation timeline straight from telemetry. Same shape as
 * the other *Info records (status + data), minus a state enum — the solenoid is binary. */

/** @brief Solenoid status (8-bit): presence + continuity + open/closed state. */
struct SolenoidStatus {
    uint8_t detected   : 1;  /**< The solenoid is present (SOL_VALVE_DET active). */
    uint8_t open       : 1;  /**< Driven open (coil energised); only ever set while in the Unsafe state. */
    uint8_t continuity : 1;  /**< Continuity-line state (SOL_VALVE_CONT) — lit whenever detected; surfaces the indicator state to the GS. */
    uint8_t reserved   : 5;  /**< Padding; room for more solenoid flags later. */
};
static_assert(sizeof(SolenoidStatus) == 1, "SolenoidStatus must be exactly 1 byte (on the wire)");

/** @brief The solenoid's reported status + the actuation timeline (last open/close ticks). */
struct SolenoidInfo {
    SolenoidStatus status;           /**< Presence (detected) + open/closed state (open). */
    uint8_t        reserved[3];      /**< Aligns the timestamps to 4 bytes; keeps the record packed. */
    uint32_t       last_opened_ms;   /**< Tick of the last open edge (0 = never opened since boot). */
    uint32_t       last_closed_ms;   /**< Tick of the last close edge (0 = never closed since boot). */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(SolenoidInfo) == 4 + 2 * sizeof(uint32_t),
              "SolenoidInfo has implicit padding — keep status/reserved at 4 bytes then the two timestamps");
