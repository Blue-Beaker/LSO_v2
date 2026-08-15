#include "QueuedMusicTracker.hpp"

#include "../utils/Utils.hpp"

void QueuedMusicTracker::tickFloatSeconds(float const tickLengthSeconds) {
    // LOG_MOD_DEBUG("tickFloatSeconds {}", tickLengthSeconds);
    float const tickLengthMsFloat = tickLengthSeconds*1000+partialMs;

    int const tickLengthMs = floor(tickLengthMsFloat);
    partialMs = tickLengthMsFloat-tickLengthMs;

    tickMs(tickLengthMs);
}

void QueuedMusicTracker::tickMs(int const tickLengthMs) {
    // LOG_MOD_DEBUG("tickMs {}, queued={}", tickLengthMs, m_queuedMusic.size());
    std::set<int> channelsToRemove;

    std::lock_guard lock(mutex);

    callingInTracker=true;
    for (auto& [channelID, m] : queuedMusic) {
        auto& music = m;

        // Make sure to pause
        // TODO: find out when to pause the audio correctly, instead of invoking this on every step (very inefficient)
        // TODO: also, starting music is not paused correctly. find out how to pause this as well.
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
        queuedMusic.erase(channelID);
    }
    callingInTracker=false;
}

bool QueuedMusicTracker::hasChannel(int const channelID) const {
    return queuedMusic.contains(channelID);
}

void QueuedMusicTracker::clearChannel(int channel){
    std::lock_guard lock(mutex);
    queuedMusic.erase(channel);
    LOG_MOD_DEBUG("Cleared queued music in channel {}", channel);
}

void QueuedMusicTracker::queueChannel(int channel, int musicID, int timeRemainingMs) {
    std::lock_guard lock(mutex);
    queuedMusic[channel]=QueuedMusic(timeRemainingMs, musicID);
    LOG_MOD_DEBUG("Queued music in channel {}, time={}, queuedCount={}", channel, timeRemainingMs, queuedMusic.size());
}

void QueuedMusicTracker::pauseAll() {
    std::lock_guard lock(mutex);
    for (auto& [channelID, music] : queuedMusic) {
        callingInTracker=true;
        FMODAudioEngine::get()->pauseMusic(channelID);
        callingInTracker=false;
    }
}

void QueuedMusicTracker::clear() {
    std::lock_guard lock(mutex);
    queuedMusic.clear();
}