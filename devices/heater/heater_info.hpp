#pragma once

#include <cstdint>

/* Telemetry unit for the FCU heater, owned and kept current by the heater: its status
 * (on/off state) and the ticks of the last on/off edges. It rides the low-rate
 * FcuExtendedSystemState so the ground station reads whether the heater is driven on and
 * the exact actuation timeline straight from telemetry. Same shape as the other *Info
 * records (status + data), minus a state enum and a presence bit — the heater is a bare
 * on/off output with no detect line. */

/** @brief Heater status (8-bit): on/off state. */
struct HeaterStatus {
    uint8_t on       : 1;  /**< Driven on (output energised). */
    uint8_t reserved : 7;  /**< Padding; room for more heater flags later. */
};
static_assert(sizeof(HeaterStatus) == 1, "HeaterStatus must be exactly 1 byte (on the wire)");

/** @brief The heater's reported status + the actuation timeline (last on/off ticks). */
struct HeaterInfo {
    HeaterStatus status;          /**< On/off state (on). */
    uint8_t      reserved[3];     /**< Aligns the timestamps to 4 bytes; keeps the record packed. */
    uint32_t     last_on_ms;      /**< Tick of the last on edge (0 = never driven on since boot). */
    uint32_t     last_off_ms;     /**< Tick of the last off edge (0 = never driven off since boot). */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(HeaterInfo) == 4 + 2 * sizeof(uint32_t),
              "HeaterInfo has implicit padding — keep status/reserved at 4 bytes then the two timestamps");
