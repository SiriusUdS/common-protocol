#pragma once

#include <cstdint>

/* ------------------------------------------------------------------------- *
 * Global filling-station state machine states — the on-the-wire SSOT.
 *
 * logic::control::State is the single source of truth, mirroring how CommandType
 * is the SSOT for command ids: the enumerator value IS the wire/telemetry
 * encoding shared with the ground station (carried in EthernetHeader.sender_state
 * and the SetState payload). Never spell these values anywhere else.
 * ------------------------------------------------------------------------- */

namespace logic::control {

/**
 * @brief The board's global state-machine state.
 *
 * Underlying type is uint8_t to match the wire/telemetry encoding exactly.
 */
enum class State : uint8_t {
    Init   = 0x00,  /**< Power-on / boot init. */
    Safe   = 0x01,  /**< People are near the system: ANY action is hazardous, so none is
                         permitted. The only exit is Unsafe (the area is declared clear). */
    Unsafe = 0x02,  /**< No people are near the system: it may be armed and operated
                         (propellant operations permitted). */
    Abort  = 0x03,  /**< Abort sequence engaged. */
    Error  = 0x04,  /**< Fault latched. */
    Ignite = 0x05,  /**< Ignition sequence. */
    Launch = 0x06,  /**< Launch sequence. */
    Test   = 0x07,  /**< Bench/diagnostics. */
};

} // namespace logic::control
