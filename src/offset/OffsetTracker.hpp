#pragma once

// Tracks what channels has offset applied. The 'channel' is the internal channels used in FMODAudioEngine, not channels set in Song Triggers.
class OffsetTracker final {
    std::unordered_set<int> channelsWithOffset;
public:
    static OffsetTracker& get() {
        static OffsetTracker s_tracker;
        return s_tracker;
    }
    // Check whether the channel has an offset set before, to avoid multiple offsets applied to a single music.
    [[nodiscard]] bool getHasOffset(int const channel) const {
        return channelsWithOffset.contains(channel);
    }
    // Called in music hooks with offset applied
    void setHasOffset(int const channel) {
        channelsWithOffset.insert(channel);
    }
    // Called in music hooks without offset applied
    void setNoOffset(int const channel) {
        channelsWithOffset.erase(channel);
    }
    // Called when quitting the level
    void clear() {
        channelsWithOffset.clear();
    }
};
