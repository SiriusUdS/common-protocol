#pragma once

#include <cstdint>

#include "peripherals/storage/storage_state.hpp"
#include "peripherals/storage/storage_status.hpp"

/* Telemetry unit for one backing store (SD card): its state, status bits and a
 * log-overrun counter, owned and kept current by the store driver. The store has no
 * per-record value to report (unlike a valve's position or an ADC's channels); the
 * counter mirrors CanInfo's dropped-frame count — it is the SD-logging pipeline's
 * dropped-record tally, surfaced here because that is where the ground station reads
 * the card's health. */

/** @brief One store's reported state + status + log-overrun count. */
struct StorageInfo {
    StorageState  state;
    StorageStatus status;
    uint16_t      overrun_count;  /**< log double-buffer overruns since boot: halves that
                                       filled before the previous one could be flushed, so
                                       records were dropped (saturating). Was a status bit. */
};

// Wire layout guard: must be packed with no implicit padding (mirrors CanInfo).
static_assert(sizeof(StorageInfo) == 4, "StorageInfo must be exactly 4 bytes");
static_assert(sizeof(StorageInfo) == 2 * sizeof(uint8_t) + sizeof(uint16_t),
              "StorageInfo has implicit padding — keep state/status then the uint16 counter");
