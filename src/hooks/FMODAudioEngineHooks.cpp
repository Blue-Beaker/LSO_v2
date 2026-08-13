#include <Geode/modify/FMODAudioEngine.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/OffsetCalculator.hpp"
#include "../offset/OffsetTracker.hpp"
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
        // The new modified values to pass to the original function. We will modify these as needed.
        int newStart = start;
        int newEnd = end;

        int totalOffset = getTotalOffset();

        // Because we can't distinguish between queued music that was already prepared (noPrepare=false) and queued music that is being prepared now (noPrepare=true), we only apply the offset to the start time when noPrepare=true. This ensures that we don't double-apply the offset in the case where the music is already prepared.
        if (totalOffset != 0 && noPrepare) {
            auto offset = applyOffset(start);
            newStart = offset.adjustedTime;
        }
        
        if(start!=newStart){
            OffsetTracker::get().setHasOffset(channelID);
            LOG_MOD_DEBUG("queueStartMusic: applying offset {} to start ({} -> {}), noPrepare={}, path='{}', musicID={}", totalOffset, start, newStart, noPrepare, path, musicID);
        }
        FMODAudioEngine::queueStartMusic(
            path, pitch, unknown, volume, loop, newStart, newEnd,
            fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
        );
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

        int newStart = start;

        auto offset = applyOffset(start);

        newStart=offset.adjustedTime;
        
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

        int totalOffset = getTotalOffset();
        if (lso::utils::offset::shouldSkipOffset()) {
            LOG_MOD_DEBUG("loadAndPlayMusic: skipping hook because not in a level");
            FMODAudioEngine::loadAndPlayMusic(path, time, musicID);
            return;
        }

        unsigned int newTime = time;

        // Apply offset
        auto offset = applyOffset(static_cast<int>(time));
        newTime = static_cast<unsigned int>(offset.adjustedTime);
        
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

        // If the offset is set before, do not re-set
        if (OffsetTracker::get().getHasOffset(music.m_channelID)) {
            FMODAudioEngine::triggerQueuedMusic(music);
            return;
        }

        auto offset = applyOffset(music.m_start);
        if (offset.adjustedTime != music.m_start) {
            OffsetTracker::get().setHasOffset(music.m_channelID);
            LOG_MOD_DEBUG("triggerQueuedMusic: applying offset to m_start ({} -> {}), channel={}",
                      music.m_start, offset.adjustedTime, music.m_channelID);
        }
        music.m_start = offset.adjustedTime;
        FMODAudioEngine::triggerQueuedMusic(music);
    }

    // ─── setMusicTimeMS ─────────────────────────────────────────────────────
    // Seeks music to a given time. Used by checkpoint restoration, pause, etc.

    void setMusicTimeMS(unsigned int ms, bool p1, int channel) {
        LOG_MOD_DEBUG("setMusicTimeMS: channelID={}", channel);
        int totalOffset = getTotalOffset();
        if(lso::utils::offset::shouldSkipOffset()){
            LOG_MOD_DEBUG("setMusicTimeMS: skipping hook because not in a level");
            FMODAudioEngine::setMusicTimeMS(ms, p1, channel);
            return;
        }

        auto offset = applyOffset(ms);
        if (offset.adjustedTime != static_cast<int>(ms)) {
            OffsetTracker::get().setHasOffset(channel);
            LOG_MOD_DEBUG("setMusicTimeMS: {} -> {} (channel={}, totalOffset={})", ms, offset.adjustedTime, channel, totalOffset);
        }
        FMODAudioEngine::setMusicTimeMS(
            static_cast<unsigned int>(offset.adjustedTime), p1, channel
        );
    }
};
