#pragma once

#include <cstdint>

#include "communication/protocol/framing/payload_type.hpp"  // PayloadType (priority is derived from it)

/* ------------------------------------------------------------------------- *
 * Custom 29-bit CAN extended identifier used by the filling-station bus.
 *
 * Overlays the 29-bit extended id of a classic CAN frame, packing the routing
 * and status metadata every board exchanges. The functional fields occupy 29
 * bits; reserved pads to a full 32-bit word. The routing/payload fields use the
 * same names and meanings as EthernetHeader so the two transports stay uniform:
 * sender_id/target_id carry BoardId values, payload_type is a PayloadType, and
 * payload_id carries a CommandType (commands) or a TelemetryType (telemetry/
 * responses). seq tags a command so its reply can be matched + retried (mirrors
 * EthernetHeader::seq); error_code is a CAN-only sender diagnostic.
 *
 * `priority` is the top bit of the 29-bit id (bit 28), so it dominates CAN bus
 * arbitration AND the FDCAN TX-queue ordering: 0 (commands/responses) always
 * out-prioritises 1 (telemetry). This keeps a command/response from queuing behind
 * the ECU's full-rate telemetry burst. It is CAN-only — EthernetHeader has no
 * equivalent (UDP has no arbitration) — and is derived from payload_type via
 * canBusPriority(), so it is not an independent source of truth. */

struct FrameCanHeader {
    uint32_t sender_id:4;     /**< BoardId that emitted the frame. */
    uint32_t target_id:4;     /**< Destination BoardId (or Broadcast). */
    uint32_t sender_state:4;  /**< Sender's device state / command sub-field. */
    uint32_t payload_type:2;  /**< Command vs telemetry vs response class (PayloadType). */
    uint32_t payload_id:6;    /**< A CommandType or a TelemetryType. */
    uint32_t seq:4;           /**< Command/response sequence: tags a command so its reply
                                   (e.g. Pong) is matched to it and retried; the receiver echoes
                                   it. 0 for telemetry. Mirrors EthernetHeader::seq. */
    uint32_t error_code:4;    /**< Error code reported by the sender (CAN-only diagnostic). */
    uint32_t priority:1;      /**< Bus/TX-queue arbitration priority: 0 high (command/response),
                                   1 low (telemetry). The id's MSB, so it dominates arbitration.
                                   Derived from payload_type via canBusPriority(). */
    uint32_t reserved:3;      /**< Unused; pads the 29-bit id to 32 bits. */
};

union CanHeader {
    FrameCanHeader frame;
    uint32_t       code;
};
static_assert(sizeof(CanHeader) == 4, "CanHeader must overlay a single 29-bit CAN id (4 bytes)");

/** @brief Bit position of FrameCanHeader::priority within the 29-bit id — the MSB, so a frame's
 *  arbitration priority is just `(id >> CAN_ID_PRIORITY_BIT) & 1`. The platform CAN driver uses
 *  this to lane TX frames without depending on the protocol enums. */
inline constexpr uint32_t CAN_ID_PRIORITY_BIT = 28;

/** @brief The bus/TX-queue priority for a payload class: 0 (high — wins arbitration and drains
 *  from the TX queue first) for Command and Response, 1 (low) for Telemetry. Single source of
 *  truth for the priority bit, so every frame builder maps payload_type the same way. */
[[nodiscard]] constexpr uint32_t canBusPriority(PayloadType type)
{
    return type == PayloadType::Telemetry ? 1u : 0u;
}
