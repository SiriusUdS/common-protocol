#pragma once

#include <cstdint>

#include "telemetry/refused_command_info.hpp"   // RefusedCommandInfo (last refused SetState + SetControlFlag)
#include "peripherals/storage/sd_write_engine_info.hpp"  // SdWriteEngineInfo (shared SD write-engine health)

/* The fields common to every board's low-rate ExtendedSystemState. Both FcuExtendedSystemState
 * and EcuExtendedSystemState embed this as their leading `base` member, so the shared prefix
 * decodes identically on the ground regardless of source board (the sibling of SystemStateBase
 * for the high-rate record).
 *
 * The control flags are the 16-bit base/per-board space (control_flags_base = the common BASE
 * byte; control_flags_board = this board's PER-BOARD byte — the GS reads both as "the flags
 * present on this board", without caring which is which). The per-board byte is 0 on a board
 * with no board-specific flags (e.g. the ECU). Board-specific peripherals (the FCU's e-match /
 * solenoid / thermocouples) are added ALONGSIDE `base` in the per-board record. */

struct ExtendedSystemStateBase {
    uint32_t           creation_timestamp_ms;  /**< Time this record was assembled. */
    uint8_t            control_flags_base;     /**< BASE control flags (bit N = ControlFlagBase value N): common to every board. */
    uint8_t            control_flags_board;    /**< PER-BOARD control flags (bit N = this board's flag value N); 0 if the board has none. */
    uint8_t            backup_status;          /**< Backup-domain retention health probed at boot (logic::control::BackupStatus). */
    uint8_t            reserved[1];            /**< Aligns refused_command_info; keeps the record packed. */
    RefusedCommandInfo refused_command_info;   /**< Last refused SetState + SetControlFlag (with the state each was refused in) + counts. */
    SdWriteEngineInfo  sd_write_engine_info;   /**< Board-wide async SD write-engine health: dropped-block count + sticky DMA-error flag. */
};

// Wire layout guard: the common prefix must be packed with no implicit padding so the ground
// station decodes it byte-for-byte.
static_assert(sizeof(ExtendedSystemStateBase) == sizeof(uint32_t)        // creation_timestamp_ms
                                               + 4                       // 2 flag bytes + backup_status + reserved[1]
                                               + sizeof(RefusedCommandInfo)
                                               + sizeof(SdWriteEngineInfo),
              "ExtendedSystemStateBase has implicit padding — add explicit reserved bytes");
