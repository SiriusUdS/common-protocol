#pragma once

#include <cstdint>
#include <optional>

/* Payload for CommandType::SetControlFlag — set a named runtime control flag. */

/**
 * @brief Which runtime control flag to set. The enumerator value IS the on-wire id
 *        (SetControlFlagFrame::flag) — one SSOT for every transport. Add flags here
 *        (e.g. arming/recording toggles the GS drives at runtime).
 */
enum class ControlFlag : uint8_t {
    PersistingData = 0x00,  /**< Persist telemetry records to the SD card. When off, records still
                                 drain from the buffer but are discarded (keeps the card from filling). */
};
static_assert(sizeof(ControlFlag) == 1, "ControlFlag must be exactly 1 byte (on the wire)");

/** @brief SetControlFlag payload (2 bytes): which flag, and its new on/off value. */
struct SetControlFlagFrame {
    ControlFlag flag;   /**< Which control flag. */
    uint8_t     value;  /**< 0 = off / disarm, non-zero = on / arm. */
};
static_assert(sizeof(SetControlFlagFrame) == 2, "SetControlFlagFrame must be 2 packed bytes");

/**
 * @brief Validate a raw on-wire id and recover the control flag it denotes.
 * @return The ControlFlag, or std::nullopt if @p id is not a known flag.
 */
[[nodiscard]] constexpr std::optional<ControlFlag> toControlFlag(uint8_t id)
{
    switch (static_cast<ControlFlag>(id)) {
        case ControlFlag::PersistingData:
            return static_cast<ControlFlag>(id);
    }
    return std::nullopt;
}
