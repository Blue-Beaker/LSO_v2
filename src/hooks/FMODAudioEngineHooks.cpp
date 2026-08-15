#include <Geode/modify/FMODAudioEngine.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/OffsetTracker.hpp"
#include "../offset/QueuedMusicTracker.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

class $modify(MyFMODAudioEngine, FMODAudioEngine) {
    // Use Late priority so jukebox (and other mods) can process the call first
    // We need to hook later than other mods to apply offsets to their replaced start time
    static void onModify(auto& self) {
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::queueStartMusic",
            Priority::VeryLate
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::startMusic",
            Priority::VeryLate
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::loadAndPlayMusic",
            Priority::VeryLate
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::triggerQueuedMusic",
            Priority::VeryLate
        );
        (void)self.setHookPriorityPost(
            "FMODAudioEngine::setMusicTimeMS",
            Priority::VeryLate
        );
    }

    // musicID - channel set in song trigger, musicID = -1-(channel), or initial channel (musicID=0)
    // channelID - internal channel ID, may be 1,2,3 etc... when triggered by a song trigger, or allocated randomly for the level-initial music
    void queueStartMusic(gd::string path, float pitch,
                         float unknown, float volume, bool loop,
                         int start, int end, int fadeIn,
                         int fadeOut, int musicID, bool p10,
                         int channelID, bool noPrepare,
                         bool dontReset) {

        LOG_MOD_DEBUG("queueStartMusic: path={}, musicID={}, channelID={}",path,musicID,channelID);
        // When not in a level, don't apply any offset
        if (QueuedMusicTracker::get().getCallingInTracker() || lso::utils::offset::shouldSkipOffset()) {
            LOG_MOD_DEBUG("queueStartMusic: skipping hook");
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
        int offset = OffsetController::get().getCurrentLevelOffset();
        // When noPrepare=false (PREP), do not apply offset at this time
        if (offset != 0 && noPrepare) {
            newStart = start+offset;
            newEnd = end+offset;
        }
        
        if(start!=newStart){
            OffsetTracker::get().setHasOffset(channelID);
            LOG_MOD_DEBUG("queueStartMusic: applying offset {} to start ({} -> {}), noPrepare={}, path='{}', musicID={}", offset, start, newStart, noPrepare, path, musicID);
        }
        // Does not actually start the music, so leave the negative offset fix to the later hooks
        FMODAudioEngine::queueStartMusic(
            path, pitch, unknown, volume, loop, newStart, newEnd,
            fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
        );
    }

    // Triggers a queued music. usually called after queueStartMusic.
    // To avoid the offset from being applied twice, use OffsetTracker to tell whether the channel has set offset before.
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

        bool const has_offset = OffsetTracker::get().getHasOffset(music.m_channelID);
        if (!has_offset) {
            int const offset = OffsetController::get().getCurrentLevelOffset();
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

        if (lso::config::isNegativeOffsetFixEnabled() && newStart+m_musicOffset<0) {
            // Compensate the global offset
            music.m_start = -m_musicOffset;
            FMODAudioEngine::triggerQueuedMusic(music);
            pauseAndQueueChannel(music.m_channelID,music.m_musicID,-newStart);
        }else {
            music.m_start = newStart;
            music.m_end = newEnd;
            FMODAudioEngine::triggerQueuedMusic(music);
        }
    }

    // Seeks music to a given time. Used by checkpoint restoration, pause, and edit song triggers.
    // Also in PlayLayer::prepareMusic
    void setMusicTimeMS(unsigned int ms, bool p1, int musicID) {
        LOG_MOD_DEBUG("setMusicTimeMS: musicID={}", musicID);

        if(QueuedMusicTracker::get().getCallingInTracker() || lso::utils::offset::shouldSkipOffset()){
            LOG_MOD_DEBUG("setMusicTimeMS: skipping hook");
            FMODAudioEngine::setMusicTimeMS(ms, p1, musicID);
            return;
        }

        int channelID = FMODAudioEngine::getMusicChannelID(musicID);
        QueuedMusicTracker::get().clearChannel(channelID);

        int const offset = OffsetController::get().getCurrentLevelOffset();
        int newStart=ms+offset;

        if (offset!=0) {
            OffsetTracker::get().setHasOffset(channelID);
            LOG_MOD_DEBUG("setMusicTimeMS: {} -> {} (channelID={}, levelOffset={})", ms, newStart, channelID, OffsetController::get().getCurrentLevelOffset());
        }
        if (lso::config::isNegativeOffsetFixEnabled() && newStart+m_musicOffset<0) {
            // Compensate the global offset
            FMODAudioEngine::setMusicTimeMS(-m_musicOffset, p1, musicID);
            pauseAndQueueChannel(channelID,musicID,-newStart);
        }else {
            FMODAudioEngine::setMusicTimeMS(
                newStart, p1, musicID
            );
        }
    }

    // Convenience method
    void pauseAndQueueChannel(int const channelID, int const musicID, int const timeRemainingMs) {
        FMODAudioEngine::pauseMusic(channelID);
        QueuedMusicTracker::get().queueChannel(channelID, musicID,timeRemainingMs);
    }

};
