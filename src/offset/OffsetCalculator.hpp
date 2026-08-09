#pragma once

#include <Geode/Geode.hpp>
#include "../utils/Utils.hpp"

using namespace geode::prelude;

/**
 * Result of applying offset to a time value (usually milliseconds).
 */
struct OffsetResult {
    // The adjusted time value with offset applied
    int adjustedTime = 0;
    // The remainder (paddedLengthMs + offset), used for padded file compensation
    int remainder = 0;
    // The padded length (calculated with lso::utils::offset::calculatePaddedLength), used for padded file naming
    int paddedLengthMs = 0;
};

/**
 * Applies the current song offset to a given time value (in milliseconds).
 *
 * Behaviour depends on whether the track uses a padded audio file:
 *  - Padded (Negative Offset with workaround): time += (padding+offset) (skip the prepended silence)
 *  - Not padded: time += offset, clamped to 0
 *
 * @param timeMs   The original time value in milliseconds
 * @param isPadded Whether the track is using a padded audio file
 * @return OffsetResult with the adjusted time
 */
OffsetResult applyOffset(int timeMs, bool isPadded = false) {

    int totalOffset = getTotalOffset();
    OffsetResult result;
    result.adjustedTime = timeMs;

    if (isPadded) {
        result.paddedLengthMs = lso::utils::offset::calculatePaddedLength(totalOffset);
        result.remainder = result.paddedLengthMs + totalOffset;
        result.adjustedTime = timeMs + result.remainder;
    } else if (totalOffset != 0) {
        result.adjustedTime = timeMs + totalOffset;
        if (result.adjustedTime < 0) result.adjustedTime = 0;
    }

    return result;
}

/**
 * Overload for unsigned int time values.
 */
inline OffsetResult applyOffset(unsigned int timeMs, bool isPadded = false) {
    return applyOffset(static_cast<int>(timeMs), isPadded);
}
