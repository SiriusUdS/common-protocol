#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "system/crc32_polynomial.hpp"   // logic::data_integrity::CRC32_POLYNOMIAL_REFLECTED

/* ------------------------------------------------------------------------- *
 * On-disk block footer for the SD telemetry logs (data_fast/slow/ext.bin).
 *
 * Every block written to the card ends with a footer: a run of repeated MAGIC words that
 * fills the block out to a fixed 12-byte trailer at the very end. The trailer carries the
 * saving timestamp, the payload length (the records that precede the footer), and a CRC-32
 * over everything before it. A reader recovers block boundaries after local corruption by
 * scanning for the MAGIC and validating the trailer's CRC.
 *
 * Layout of one written block of `total` bytes:
 *   [ payload : payload_bytes ][ MAGIC x k ][ saved_ms ][ payload_bytes ][ crc32 ]
 *                              \_________ footer (>= SD_BLOCK_MIN_FOOTER_BYTES) _________/
 *   - the MAGIC fill spans [payload_bytes, total - 12), so the block is fully used (no dead
 *     padding); for the fixed 4096-byte fast block it absorbs the slack between the last whole
 *     record and the trailer. For the variable slow/ext writes it is a single MAGIC word.
 *   - crc32 (zlib / reflected-0xEDB88320, the same variant as the telemetry-frame CRC) covers
 *     [0, total - 4): payload + MAGIC fill + saved_ms + payload_bytes.
 *
 * This header is self-contained (it carries its own software CRC), so an offline reader can
 * parse the files with no firmware/platform dependency. The firmware writer stamps the footer
 * with stampSdBlockFooter(), passing its hardware-backed CRC seam; both must agree on the zlib
 * variant (they do — see crc32_host.cpp / the platform CRC bring-up).
 *
 * Reading: all three files (data_fast/slow/ext.bin) are streams of fixed SD_LOG_BLOCK_BYTES
 * blocks — chunk by that size and call sdBlockVerify() on each chunk. Its payload holds a whole
 * number of that file's records (high-rate SystemState, averaged SystemState, or
 * ExtendedSystemState respectively). After local corruption, sdBlockScanForMagic() resyncs to a
 * block boundary (the MAGIC fill sits right after each block's records).
 * ------------------------------------------------------------------------- */

namespace logic::telemetry {

/** @brief Sector-aligned size of one fast-log block (data_fast.bin), footer included. Shared so
 *         both boards' buffer rings, this recorder, and any reader agree. */
inline constexpr std::size_t SD_LOG_BLOCK_BYTES = 4096;

/** @brief Footer marker word, repeated to fill a block out to its trailer; little-endian on disk. */
inline constexpr uint32_t SD_BLOCK_MAGIC = 0x5D10F007u;

/** @brief The fixed trailer at the very end of every written block. */
struct SdBlockTrailer {
    uint32_t saved_ms;       /**< Tick when the block was written (the saving timestamp). */
    uint32_t payload_bytes;  /**< Length of the record payload that precedes the footer. */
    uint32_t crc32;          /**< zlib CRC-32 over [0, total - 4) — everything but this word. */
};
static_assert(sizeof(SdBlockTrailer) == 12, "SdBlockTrailer must be exactly 12 bytes on disk");

/** @brief Trailer size (the footer is this plus at least one MAGIC word). */
inline constexpr std::size_t SD_BLOCK_TRAILER_BYTES = sizeof(SdBlockTrailer);

/** @brief Smallest footer: one MAGIC word + the trailer. Producers reserve at least this many
 *         bytes of each block for the footer, so the records leave room for it. */
inline constexpr std::size_t SD_BLOCK_MIN_FOOTER_BYTES = sizeof(uint32_t) + SD_BLOCK_TRAILER_BYTES;

/** @brief Most payload bytes a fast-log block can hold while leaving room for the footer. Both
 *         the telemetry buffer ring and the recorder's slow/ext accumulators flip to a new block
 *         once another record would exceed this, so the footer always fits inside the 4096. */
inline constexpr std::size_t SD_BLOCK_PAYLOAD_CAP = SD_LOG_BLOCK_BYTES - SD_BLOCK_MIN_FOOTER_BYTES;

namespace detail {

inline void putU32le(uint8_t* p, uint32_t v)
{
    p[0] = static_cast<uint8_t>(v);
    p[1] = static_cast<uint8_t>(v >> 8);
    p[2] = static_cast<uint8_t>(v >> 16);
    p[3] = static_cast<uint8_t>(v >> 24);
}

[[nodiscard]] inline uint32_t getU32le(const uint8_t* p)
{
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

} // namespace detail

/** @brief Software zlib / reflected CRC-32 (bit-at-a-time), matching the firmware's hardware
 *         CRC and the host logic::data_integrity::crc32 seam. Self-contained for offline
 *         readers; the writer instead passes its own (hardware) CRC to stampSdBlockFooter(). */
[[nodiscard]] inline uint32_t sdBlockCrc32(const uint8_t* data, std::size_t length_bytes)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (std::size_t i = 0; i < length_bytes; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            const uint32_t mask = ~(crc & 1u) + 1u;
            crc = (crc >> 1) ^ (logic::data_integrity::CRC32_POLYNOMIAL_REFLECTED & mask);
        }
    }
    return ~crc;
}

/**
 * @brief Stamp the footer into @p block: fill [payload_bytes, total - 12) with the repeating
 *        MAGIC (phase-aligned to the block, so words land on 4-byte boundaries), then write the
 *        trailer (saved_ms, payload_bytes, crc) into the last 12 bytes. The CRC is computed by
 *        @p crc_fn over [0, total - 4). Requires payload_bytes + SD_BLOCK_MIN_FOOTER_BYTES <= total.
 * @tparam Crc  uint32_t(const uint8_t*, std::size_t) — the writer's CRC (the HW seam in firmware).
 */
template <typename Crc>
inline void stampSdBlockFooter(std::span<uint8_t> block, std::size_t payload_bytes,
                               uint32_t saved_ms, Crc crc_fn)
{
    const std::size_t total       = block.size();
    uint8_t* const    data        = block.data();
    const std::size_t trailer_off = total - SD_BLOCK_TRAILER_BYTES;

    // MAGIC fill [payload_bytes, trailer_off). Phase to the absolute block offset so each
    // aligned word reads back as SD_BLOCK_MAGIC (payload_bytes is a whole number of records).
    for (std::size_t i = payload_bytes; i < trailer_off; ++i) {
        data[i] = static_cast<uint8_t>(SD_BLOCK_MAGIC >> (8u * (i & 3u)));
    }
    detail::putU32le(data + trailer_off,     saved_ms);
    detail::putU32le(data + trailer_off + 4, static_cast<uint32_t>(payload_bytes));
    detail::putU32le(data + trailer_off + 8, crc_fn(data, total - sizeof(uint32_t)));
}

/** @brief A verified block's payload (the records) plus its decoded trailer. */
struct SdBlockView {
    std::span<const uint8_t> payload;
    SdBlockTrailer           trailer;
};

/**
 * @brief Parse + verify a complete block whose trailer is its last 12 bytes: read the trailer,
 *        recompute the CRC over [0, total - 4), and check it. The MAGIC fill is validated
 *        implicitly (it is inside the CRC'd region). Use this on each fixed SD_LOG_BLOCK_BYTES
 *        chunk of data_fast.bin.
 * @return The payload [0, payload_bytes) + trailer, or std::nullopt if the block is too small,
 *         payload_bytes is out of range, or the CRC fails.
 */
[[nodiscard]] inline std::optional<SdBlockView> sdBlockVerify(std::span<const uint8_t> block)
{
    const std::size_t total = block.size();
    if (total < SD_BLOCK_MIN_FOOTER_BYTES) {
        return std::nullopt;
    }
    const uint8_t* const data        = block.data();
    const std::size_t    trailer_off = total - SD_BLOCK_TRAILER_BYTES;

    SdBlockTrailer t;
    t.saved_ms      = detail::getU32le(data + trailer_off);
    t.payload_bytes = detail::getU32le(data + trailer_off + 4);
    t.crc32         = detail::getU32le(data + trailer_off + 8);

    if (t.payload_bytes + SD_BLOCK_MIN_FOOTER_BYTES > total) {
        return std::nullopt;  // payload would overlap the footer
    }
    if (sdBlockCrc32(data, total - sizeof(uint32_t)) != t.crc32) {
        return std::nullopt;
    }
    return SdBlockView{ block.subspan(0, t.payload_bytes), t };
}

/**
 * @brief Scan forward from @p offset for the next 4-aligned SD_BLOCK_MAGIC word — the resync
 *        primitive after local corruption.
 * @return The byte offset of the magic word, or data.size() if none is found.
 */
[[nodiscard]] inline std::size_t sdBlockScanForMagic(std::span<const uint8_t> data, std::size_t offset)
{
    std::size_t i = (offset + 3u) & ~static_cast<std::size_t>(3);   // round up to 4-aligned
    for (; i + sizeof(uint32_t) <= data.size(); i += sizeof(uint32_t)) {
        if (detail::getU32le(data.data() + i) == SD_BLOCK_MAGIC) {
            return i;
        }
    }
    return data.size();
}

} // namespace logic::telemetry
