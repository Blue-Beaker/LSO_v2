#include <Geode/modify/FMODAudioEngine.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/OffsetCalculator.hpp"
#include "../offset/OffsetTracker.hpp"
#include "../offset/QueuedMusicTracker.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

class $modify(MyFMODAudioEngine, FMODAudioEngine) {
    // Use Late priority so jukebox (and other mods) can process the call first
    static void onModify(auto& self) {
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::queueStartMusic",
            Priority::Late
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::startMusic",
            Priority::Late
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::loadAndPlayMusic",
            Priority::Late
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::triggerQueuedMusic",
            Priority::Late
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::setMusicTimeMS",
            Priority::Late
        );
    }

    void queueStartMusic(gd::string path, float pitch,
                         float unknown, float volume, bool loop,
                         int start, int end, int fadeIn,
                         int fadeOut, int musicID, bool p10,
                         int channelID, bool noPrepare,
                         bool dontReset) {

        LOG_MOD_DEBUG("queueStartMusic: path={}, musicID={}, channelID={}",path,musicID,channelID);
        // When not in a level, don't apply any offset
        if (lso::utils::offset::shouldSkipOffset()) {
            LOG_MOD_DEBUG("queueStartMusic: skipping hook because not in a level");
            FMODAudioEngine::queueStartMusic(
                path, pitch, unknown, volume, loop, start, end,
                fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
            );
            return;
        }

        QueuedMusicTracker::get().clearChannel(channelID);

        int newStart = start;
        int newEnd = end;

        // The new modified values to pass to the original function. We will modify these as needed.
        int offset = getCurrentLevelOffset();
        // When noPrepare=false (PREP), do not apply offset at this time
        if (offset != 0 && noPrepare) {
            newStart = start+offset;
            newEnd = end+offset;
        }
        
        if(start!=newStart){
            OffsetTracker::get().setHasOffset(channelID);
            LOG_MOD_DEBUG("queueStartMusic: applying offset {} to start ({} -> {}), noPrepare={}, path='{}', musicID={}", offset, start, newStart, noPrepare, path, musicID);
        }
        // if (lso::config::isNegativeOffsetFixEnabled() && newStart<0) {
        //     FMODAudioEngine::queueStartMusic(
        //         path, pitch, unknown, volume, loop, 0, newEnd,
        //         fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
        //     );
        //     pauseAndQueueChannel(channelID,-newStart);
        // }else {
            FMODAudioEngine::queueStartMusic(
                path, pitch, unknown, volume, loop, newStart, newEnd,
                fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
            );
        // }
    }

    // Called by PlayLayer::startMusic() on practice mode respawn / resetLevel.
    // queueStartMusic is NOT called in that path, so we need this hook.
    void startMusic(int start, int end, int fadeIn, int fadeOut,
                    bool loop, int musicID, bool noResume, bool dontReset) {
        LOG_MOD_DEBUG("startMusic: musicID={}",musicID);

        if (lso::utils::offset::shouldSkipOffset()) {
            LOG_MOD_DEBUG("startMusic: skipping hook because not in a level");
            FMODAudioEngine::startMusic(start, end, fadeIn, fadeOut, loop, musicID, noResume, dontReset);
            return;
        }
        int channelID = FMODAudioEngine::getMusicChannelID(musicID);
        QueuedMusicTracker::get().clearChannel(channelID);

        int offset = getCurrentLevelOffset();
        int newStart=start+offset;
        
        if(start!=newStart){
            OffsetTracker::get().setHasOffset(getMusicChannelID(musicID));
            LOG_MOD_DEBUG("startMusic: applying offset ({} -> {}), musicID={}", start, newStart, musicID);
        }

        FMODAudioEngine::startMusic(
            newStart, end, fadeIn, fadeOut, loop,
            musicID, noResume, dontReset
        );
    }

    // Loads and immediately starts playback at a given time position.
    // Called by song triggers mid-level to switch music.
    void loadAndPlayMusic(gd::string path, unsigned int time, int musicID) {
        LOG_MOD_DEBUG("loadAndPlayMusic: path={}, musicID={}", path, musicID);

        if (lso::utils::offset::shouldSkipOffset()) {
            LOG_MOD_DEBUG("loadAndPlayMusic: skipping hook because not in a level");
            FMODAudioEngine::loadAndPlayMusic(path, time, musicID);
            return;
        }

        int channelID = FMODAudioEngine::getMusicChannelID(musicID);
        QueuedMusicTracker::get().clearChannel(channelID);

        unsigned int newTime = time;

        // Apply offset
        int offset = getCurrentLevelOffset();
        newTime = time+offset;
        
        if(time!=newTime){
            OffsetTracker::get().setHasOffset(getMusicChannelID(musicID));
            LOG_MOD_DEBUG("loadAndPlayMusic: applying offset ({} -> {}), path='{}'", time, newTime, path);
        }
        
        FMODAudioEngine::loadAndPlayMusic(
            path,
            newTime,
            musicID
        );
    }

    // ─── triggerQueuedMusic ─────────────────────────────────────────────────
    // Activates a queued music entry. Called when:
    //   queueStartMusic(noPrepare=false) finishes prep: m_start already set
    //   Song trigger directly constructs FMODQueuedMusic: m_start is raw
    //
    // We cannot distinguish between these, so we always apply offset.

    void triggerQueuedMusic(FMODQueuedMusic music) {
        LOG_MOD_DEBUG("triggerQueuedMusic: musicID={}, channelID={}", music.m_musicID,music.m_channelID);

        if(lso::utils::offset::shouldSkipOffset()){
            LOG_MOD_DEBUG("triggerQueuedMusic: skipping hook because not in a level");
            FMODAudioEngine::triggerQueuedMusic(music);
            return;
        }

        QueuedMusicTracker::get().clearChannel(music.m_channelID);
        // If the offset is set before, do not re-set
        int newStart = music.m_start;
        int newEnd = music.m_end;

        bool has_offset = OffsetTracker::get().getHasOffset(music.m_channelID);
        if (!has_offset) {
            int offset = getCurrentLevelOffset();
            newStart = music.m_start+offset;
            newEnd = music.m_end+offset;
        }

        if (newStart != music.m_start) {
            OffsetTracker::get().setHasOffset(music.m_channelID);
        }
        if (has_offset || newStart != music.m_start) {
            LOG_MOD_DEBUG("triggerQueuedMusic: applying offset to m_start ({} -> {}), channel={}",
                      music.m_start, newStart, music.m_channelID);
        }

        if (lso::config::isNegativeOffsetFixEnabled() && newStart<0) {
            music.m_start = 0;
            FMODAudioEngine::triggerQueuedMusic(music);
            pauseAndQueueChannel(music.m_channelID,-newStart);
        }else {
            music.m_start = newStart;
            FMODAudioEngine::triggerQueuedMusic(music);
        }
    }

    // ─── setMusicTimeMS ─────────────────────────────────────────────────────
    // Seeks music to a given time. Used by checkpoint restoration, pause, etc.

    void setMusicTimeMS(unsigned int ms, bool p1, int musicID) {
        LOG_MOD_DEBUG("setMusicTimeMS: musicID={}", musicID);

        if(lso::utils::offset::shouldSkipOffset()){
            LOG_MOD_DEBUG("setMusicTimeMS: skipping hook because not in a level");
            FMODAudioEngine::setMusicTimeMS(ms, p1, musicID);
            return;
        }

        int channelID = FMODAudioEngine::getMusicChannelID(musicID);
        QueuedMusicTracker::get().clearChannel(channelID);

        int offset = getCurrentLevelOffset();
        int newStart=ms+offset;

        if (offset!=0) {
            OffsetTracker::get().setHasOffset(channelID);
            LOG_MOD_DEBUG("setMusicTimeMS: {} -> {} (channelID={}, levelOffset={}, totalOffset={})", ms, newStart, channelID, getCurrentLevelOffset(), getTotalOffset());
        }
        if (lso::config::isNegativeOffsetFixEnabled() && newStart<0) {
            FMODAudioEngine::setMusicTimeMS(
                0, p1, musicID
                );
            pauseAndQueueChannel(channelID,-newStart);
        }else {
            FMODAudioEngine::setMusicTimeMS(
                newStart, p1, musicID
            );
        }
    }

    void pauseAndQueueChannel(int channel, int timeRemainingMs) {
        FMODAudioEngine::pauseMusic(channel);
        QueuedMusicTracker::get().queueChannel(channel,timeRemainingMs);
    }
};
