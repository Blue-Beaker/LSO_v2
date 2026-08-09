#pragma once

#include "../utils/Utils.hpp"

#include <unordered_set>
#include <mutex>

/**
 * Tracks which audio tracks are currently using padded audio files.
 *
 * Tracks by both musicID and channelID so that any hook method can query
 * regardless of which identifier it has available:
 *  - queueStartMusic  -> sets by musicID and channelID
 *  - startMusic       -> queries by musicID
 *  - loadAndPlayMusic -> queries by musicID
 *  - triggerQueuedMusic -> queries by channelID (via m_channelID)
 *  - setMusicTimeMS   -> queries by channelID (via channel param)
 *
 * Thread-safe for concurrent access from audio/pregen threads.
 */
struct PaddedTrackTracker {
    // Mark a track as using padded audio (by both musicID and channelID).
    // channelID may be 0 (default channel) - still tracked so that hooks
    // like setMusicTimeMS(channel=0) can correctly detect padded state.
    void setPadded(int channelID) {
        std::lock_guard lock(m_mutex);
        m_byChannelID.insert(channelID);
    }

    // Mark a track as NOT using padded audio (by both musicID and channelID).
    void setOriginal(int channelID) {
        std::lock_guard lock(m_mutex);
        m_byChannelID.erase(channelID);
    }

    // Check by channelID.
    bool isPaddedByChannel(int channelID) const {
        std::lock_guard lock(m_mutex);
        return m_byChannelID.contains(channelID);
    }

    void setPaddedFlag(bool padded, int channelID) {
        if(padded){
            setPadded(channelID);
        }else{
            setOriginal(channelID);
        }
    }

    // Clear all tracking (e.g. on level exit).
    void clear() {
        std::lock_guard lock(m_mutex);
        m_byChannelID.clear();
    }
    bool m_isPaddedNow = false;

private:
    mutable std::mutex m_mutex;
    std::unordered_set<int> m_byChannelID;
};

// Global instance.
inline PaddedTrackTracker s_paddedTracks;
