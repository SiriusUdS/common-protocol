#pragma once

#include <cstdint>

/* Telemetry unit for the board's single asynchronous SD write engine (the level-3 datalogger's
 * shared DMA arbiter). One physical card, one SDMMC peripheral, one engine — so this is a
 * board-wide health record, not a per-file one (unlike StorageInfo, which each log file reports).
 * It rides the low-rate ExtendedSystemStateBase, since these are slow-moving pipeline counters
 * the ground station does not need thousands of times a second.
 *
 * Distinct from StorageInfo.overrun_count (the telemetry double-buffer ring overrun, on the
 * high-rate record): that counts records dropped before a half could be flushed; THIS counts whole
 * blocks dropped because the engine's write ring was full when enqueue() ran — i.e. the card could
 * not be drained as fast as the recorder produced. `errored` is the engine's sticky DMA-fault flag
 * (a write that failed to start or errored mid-transfer). `card_detected` is the physical
 * card-present signal (the SD_DETECT socket switch) — a card-absent reading explains a stalled or
 * never-started log even when the pipeline counters look clean. */

/** @brief The SD write engine's reported health: dropped-block count + sticky DMA-error flag +
 *         physical card-present. One physical card / one engine, so this is board-wide. */
struct SdWriteEngineInfo {
    uint16_t overrun_count;  /**< Blocks dropped because the engine write ring was full when
                                  enqueue() ran (saturating). Distinct from the telemetry
                                  double-buffer overrun on the high-rate StorageInfo. */
    uint8_t  errored;        /**< Sticky: a DMA write failed to start or errored mid-transfer (0/1). */
    uint8_t  card_detected;  /**< A card is seated in the socket (SD_DETECT line active): 1 present,
                                  0 absent. Board-wide (one physical card). Also keeps the record a
                                  4-byte multiple so the ExtendedSystemStateBase stays packed. */
};

// Wire layout guard: must be packed with no implicit padding (a 4-byte multiple keeps the
// 4-aligned ExtendedSystemStateBase that embeds it packed too).
static_assert(sizeof(SdWriteEngineInfo) == sizeof(uint16_t) + 2 * sizeof(uint8_t),
              "SdWriteEngineInfo has implicit padding — keep the count then the two bytes");
static_assert(sizeof(SdWriteEngineInfo) == 4, "SdWriteEngineInfo must be exactly 4 bytes");
