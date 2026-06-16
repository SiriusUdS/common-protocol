#pragma once

#include <cstdint>

/* Telemetry unit for the FCU's last refused commands, so the ground station can see which
 * commanded change was denied and from which state. Bundles the three refusal diagnostics the
 * control layer tracks — the last refused SetState (the shared transition table rejected it)
 * and the last refused SetControlFlag (an unknown flag, or a state-gated flag like the solenoid
 * valve commanded outside Unsafe). Kept as one scoped record (not loose fields) and ridden by
 * the low-rate FcuExtendedSystemState. State/flag ids are the raw on-wire values. */

struct RefusedCommandInfo {
    uint16_t set_flag_id;              /**< Refused SetControlFlag: 16-bit flag id (0xFFFF = none refused since boot). */
    uint8_t  set_flag_value;           /**< Refused SetControlFlag: requested value (0/1). */
    uint8_t  set_flag_state;           /**< Refused SetControlFlag: state the board was in when refused (State value). */
    uint8_t  set_state_from;           /**< Refused SetState: state the board was in (State value). */
    uint8_t  set_state_to;             /**< Refused SetState: requested state. Both 0 (Init) = none refused since boot. */
    uint8_t  reserved[2];              /**< Aligns the counts to 2 bytes; keeps the record packed. */
    uint16_t set_state_refused_count;  /**< Number of SetState commands refused since boot (wraps at 16 bits). */
    uint16_t set_flag_refused_count;   /**< Number of SetControlFlag commands refused since boot (wraps at 16 bits). */
    uint8_t  set_valve_id;             /**< Refused SetValvePosition: raw valve id (0xFF = none refused since boot). */
    uint8_t  set_valve_action;         /**< Refused SetValvePosition: raw action (open/close/set-%). */
    uint8_t  set_valve_value;          /**< Refused SetValvePosition: opened-% value requested. */
    uint8_t  set_valve_state;          /**< Refused SetValvePosition: state the board was in when refused (State value). */
    uint16_t set_valve_refused_count;  /**< Number of SetValvePosition commands refused since boot (wraps at 16 bits). */
    uint8_t  reserved2[2];             /**< Pads the record to a 4-byte multiple so the 4-aligned ExtendedSystemStateBase that embeds it gains no tail padding. */
};

// Wire layout guard: must be packed with no implicit padding, and a 4-byte-multiple size so the
// 4-aligned ExtendedSystemStateBase embedding it picks up no implicit tail padding of its own.
static_assert(sizeof(RefusedCommandInfo) == 8 + 2 * sizeof(uint16_t)  // flag/state ids + the two counts
                                          + 4 + sizeof(uint16_t)      // valve id+action+value+state + its count
                                          + 2,                        // reserved2
              "RefusedCommandInfo has implicit padding — keep the fields packed");
static_assert(sizeof(RefusedCommandInfo) % 4 == 0,
              "RefusedCommandInfo must be a 4-byte multiple so ExtendedSystemStateBase stays packed");
