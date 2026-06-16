#pragma once

#include <cstdint>
#include <optional>

/* Payload for CommandType::SetControlFlag — set a named runtime control flag.
 *
 * Control flags live in a 16-bit id space, split into two bytes: the LOW byte (ids 0..7)
 * is the BASE set, common to every board (e.g. the SD-card recording flags); the HIGH byte
 * (ids 8..15) is the PER-BOARD set, board-specific (e.g. the FCU solenoid valve). The two
 * are separate enums + separate runtime bitmasks (see control_flags.hpp), but they share
 * one on-wire id space so the ground station addresses any flag by a single 16-bit id — it
 * never needs to know whether a flag is base or per-board. A board maps id < 8 to its base
 * bitmask (bit = id) and id >= 8 to its per-board bitmask (bit = id - 8). */

/** @brief Global id of the first per-board flag: ids 0..7 are base, 8..15 are per-board. */
inline constexpr uint16_t CONTROL_FLAG_BOARD_OFFSET = 8;

/**
 * @brief Base control flags — common to every board. The enumerator value is the bit
 *        position in the base byte AND the low-byte on-wire id (ids 0..7).
 */
enum class ControlFlagBase : uint8_t {
    PersistingData = 0x00,  /**< Persist telemetry records to the SD card. When off, records still
                                 drain from the buffer but are discarded (keeps the card from filling). */
    FastRecording  = 0x01,  /**< While persisting: log the high-rate SystemState at the fast rate
                                 (2 kHz -> data_fast.bin) when set, or downsampled to the slow rate
                                 (100 Hz -> data_slow.bin) when clear. ExtendedSystemState (data_ext.bin)
                                 is logged regardless. No effect while PersistingData is off. */
};
static_assert(sizeof(ControlFlagBase) == 1, "ControlFlagBase must be exactly 1 byte (a bit position 0..7)");

/**
 * @brief Per-board control flags for the FCU. The enumerator value is the bit position in
 *        the per-board byte (0..7); its on-wire id is CONTROL_FLAG_BOARD_OFFSET + the value.
 */
enum class FcuControlFlag : uint8_t {
    SolenoidValve = 0x00,  /**< Command the FCU solenoid valve open (set) or closed (clear). Actuation
                                is gated to Unsafe: it only opens while set AND the board is in Unsafe,
                                and auto-closes (and the flag is cleared) on leaving Unsafe. */
    Heater        = 0x01,  /**< Command the FCU heater on (set) or off (clear). Unlike the solenoid it is
                                not state-gated: the heater follows this flag in any state (Control::serviceHeater). */
};
static_assert(sizeof(FcuControlFlag) == 1, "FcuControlFlag must be exactly 1 byte (a bit position 0..7)");

/** @brief SetControlFlag payload: the 16-bit global flag id and its new on/off value. */
struct SetControlFlagFrame {
    uint16_t flag;      /**< 16-bit global flag id: 0..7 base, 8..15 per-board (board bit = flag - 8). */
    uint8_t  value;     /**< 0 = off / clear, non-zero = on / set. */
    uint8_t  reserved;  /**< Pad to 4 bytes; keep the frame packed. */
};
static_assert(sizeof(SetControlFlagFrame) == 4, "SetControlFlagFrame must be 4 packed bytes");

/**
 * @brief Validate a base-byte on-wire id (0..7) and recover the base flag it denotes.
 * @return The ControlFlagBase, or std::nullopt if @p id is not a known base flag.
 */
[[nodiscard]] constexpr std::optional<ControlFlagBase> toControlFlagBase(uint16_t id)
{
    switch (static_cast<ControlFlagBase>(id)) {
        case ControlFlagBase::PersistingData:
        case ControlFlagBase::FastRecording:
            return static_cast<ControlFlagBase>(id);
    }
    return std::nullopt;
}

/**
 * @brief Validate a per-board-byte bit (0..7, i.e. on-wire id minus CONTROL_FLAG_BOARD_OFFSET)
 *        and recover the FCU per-board flag it denotes.
 * @return The FcuControlFlag, or std::nullopt if @p board_bit is not a known FCU flag.
 */
[[nodiscard]] constexpr std::optional<FcuControlFlag> toFcuControlFlag(uint16_t board_bit)
{
    switch (static_cast<FcuControlFlag>(board_bit)) {
        case FcuControlFlag::SolenoidValve:
        case FcuControlFlag::Heater:
            return static_cast<FcuControlFlag>(board_bit);
    }
    return std::nullopt;
}
