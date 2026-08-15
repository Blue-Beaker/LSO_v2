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

    FMODAudioEngine * engine = FMODAudioEngine::get();
    for (auto& [channelID, m] : queuedMusic) {
        auto& music = m;

        auto* channel = engine->channelForChannelID(channelID);

        music.tick(tickLengthMs);
        // LOG_MOD_DEBUG("music.tick channel={}, tickLengthMs={}, timeRemainingMs={}", channelID, tickLengthMs, music.timeRemainingMs);
        if (music.shouldStart()) {
            // Unmute the channel
            channel->setMute(false);

            engine->setMusicTimeMS(music.getStartOffset()-engine->m_musicOffset,true,music.musicID);
            engine->resumeMusic(channelID);

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