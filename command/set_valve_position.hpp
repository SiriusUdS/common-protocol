#pragma once

#include <cstdint>

#include "system/valves/fcu.hpp"   // FcuValves — the SSOT for which valve

/* Payload for CommandType::SetValvePosition — drive a valve to a position. */

/** @brief What to do to the targeted valve. */
enum class ValveCommand : uint8_t {
    Open         = 0x00,  /**< Drive fully open. @ref SetValvePositionFrame::value is ignored. */
    Close        = 0x01,  /**< Drive fully closed. @ref SetValvePositionFrame::value is ignored. */
    SetOpenedPct = 0x02,  /**< Drive to @ref SetValvePositionFrame::value percent open. */
};
static_assert(sizeof(ValveCommand) == 1, "ValveCommand must be exactly 1 byte (on the wire)");

/** @brief SetValvePosition payload (4 packed bytes). */
struct SetValvePositionFrame {
    FcuValves    valve;   /**< Which valve to drive. */
    ValveCommand action;  /**< Open / Close / SetOpenedPct. */
    uint8_t      value;   /**< Opened %, 0..100. Used only by SetOpenedPct; optional (ignored) for Open/Close. */
    uint8_t      force;   /**< 0 = normal switch-monitored move; non-zero = FORCED actuation — the
                              limit switches are bypassed for FORCED_VALVE_ACTUATION_MS, then the
                              valve auto-reverts to normal. Applies to Open/Close; ignored by
                              SetOpenedPct (a proportional hold has no switch to bypass). */
};
static_assert(sizeof(SetValvePositionFrame) == 4, "SetValvePositionFrame must be 4 packed bytes");
