#include "QueuedMusicTracker.hpp"

#include "../utils/Utils.hpp"

void QueuedMusicTracker::tickFloatSeconds(float tickLengthSeconds) {
    // LOG_MOD_DEBUG("tickFloatSeconds {}", tickLengthSeconds);
    float tickLengthMsFloat = tickLengthSeconds*1000+m_partialMs;

    int tickLengthMs = floor(tickLengthMsFloat);
    m_partialMs = tickLengthMsFloat-tickLengthMs;

    tickMs(tickLengthMs);
}

void QueuedMusicTracker::tickMs(int tickLengthMs) {
    // LOG_MOD_DEBUG("tickMs {}, queued={}", tickLengthMs, m_queuedMusic.size());
    std::set<int> channelsToRemove;

    std::lock_guard lock(m_mutex);

    m_callingInTracker=true;
    for (auto [channelID, m] : m_queuedMusic) {
        auto& music = m_queuedMusic[channelID];

        // Make sure to pause
        // if (!music.paused) {
            FMODAudioEngine::get()->pauseMusic(channelID);
            music.paused = true;
        // }

        music.tick(tickLengthMs);
        // LOG_MOD_DEBUG("music.tick channel={}, tickLengthMs={}, timeRemainingMs={}", channelID, tickLengthMs, music.timeRemainingMs);
        if (music.shouldStart()) {
            FMODAudioEngine::get()->setMusicTimeMS(music.getStartOffset(),true,music.musicID);
            FMODAudioEngine::get()->resumeMusic(channelID);
            LOG_MOD_DEBUG("resuming queued music in channel {}", channelID);
            channelsToRemove.insert(channelID);
        }
    }

    for (const int channelID : channelsToRemove) {
        m_queuedMusic.erase(channelID);
    }
    m_callingInTracker=false;
}

bool QueuedMusicTracker::hasChannel(int channelID) {
    return m_queuedMusic.contains(channelID);
}

void QueuedMusicTracker::clearChannel(int channel){
    std::lock_guard lock(m_mutex);
    m_queuedMusic.erase(channel);
    LOG_MOD_DEBUG("Cleared queued music in channel {}", channel);
}

void QueuedMusicTracker::queueChannel(int channel, int musicID, int timeRemainingMs) {
    std::lock_guard lock(m_mutex);
    m_queuedMusic[channel]=QueuedMusic(timeRemainingMs, musicID);
    LOG_MOD_DEBUG("Queued music in channel {}, time={}, queuedCount={}", channel, timeRemainingMs, m_queuedMusic.size());
}

void QueuedMusicTracker::pauseAll() {
    std::lock_guard lock(m_mutex);
    for (auto [channelID, music] : m_queuedMusic) {
        m_callingInTracker=true;
        FMODAudioEngine::get()->pauseMusic(channelID);
        m_callingInTracker=false;
    }
}

void QueuedMusicTracker::clear() {
    std::lock_guard lock(m_mutex);
    m_queuedMusic.clear();
}