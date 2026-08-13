#pragma once

class OffsetTracker {
    std::unordered_set<int> m_channels_with_offset;
    public:
    static OffsetTracker get() {
        static OffsetTracker tracker;
        return tracker;
    }

    bool getHasOffset(int channel) {
        return m_channels_with_offset.contains(channel);
    }

    void setHasOffset(int channel) {
        m_channels_with_offset.insert(channel);
    }
    void setNoOffset(int channel) {
        m_channels_with_offset.erase(channel);
    }
    void clear() {
        m_channels_with_offset.clear();
    }
};
