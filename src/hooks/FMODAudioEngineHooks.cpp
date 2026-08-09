#include <Geode/modify/FMODAudioEngine.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/OffsetCalculator.hpp"
#include "../offset/PaddedTrackTracker.hpp"
#include "../offset/negative-offset-workaround/CacheStorage.hpp"
#include "../offset/negative-offset-workaround/PaddedTrackManager.hpp"
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
        // When not in a level, don't apply any offset or redirect to padded files.
        if (lso::utils::offset::shouldSkipOffset(getTotalOffset())) {
            LOG_MOD_DEBUG("queueStartMusic: skipping hook because not in a level");
            s_paddedTracks.setOriginal(channelID);
            FMODAudioEngine::queueStartMusic(
                path, pitch, unknown, volume, loop, start, end,
                fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
            );
            return;
        }
        
        // The new modified values to pass to the original function. We will modify these as needed.
        gd::string newPath = gd::string(path);
        int newStart = start;
        int newEnd = end;

        // When in a level, apply offset and redirect to padded files if necessary.

        int totalOffset = getTotalOffset();
        // bool isMusicPadded = false;
        bool isMusicPadded = s_paddedTracks.m_isPaddedNow;

        // Negative offset with fix enabled: redirect to padded file
        if (lso::config::shouldDoNegativeOffsetWorkaround(totalOffset)) {
            auto paddedResult = PaddedTrackManager::get().getPaddedResult(totalOffset, path);
            isMusicPadded = paddedResult.isPadded;
            newPath = paddedResult.resultingPath;
        }
        
        s_paddedTracks.setPaddedFlag(isMusicPadded, channelID);

        // Because we can't distinguish between queued music that was already prepared (noPrepare=false) and queued music that is being prepared now (noPrepare=true), we only apply the offset to the start time when noPrepare=true. This ensures that we don't double-apply the offset in the case where the music is already prepared.
        if (totalOffset != 0 && noPrepare) {
            auto offset = applyOffset(start, isMusicPadded);
            newStart = offset.adjustedTime;
        }
        
        if(start!=newStart || isMusicPadded){
            LOG_MOD_DEBUG("queueStartMusic: applying offset {} to start ({} -> {}), noPrepare={}, path=('{}' -> '{}'), musicID={}, padded={}", totalOffset, start, newStart, noPrepare, path, newPath, musicID, isMusicPadded);
        }
        FMODAudioEngine::queueStartMusic(
            newPath, pitch, unknown, volume, loop, newStart, newEnd,
            fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
        );
    }

    // Called by PlayLayer::startMusic() on practice mode respawn / resetLevel.
    // queueStartMusic is NOT called in that path, so we need this hook.
    void startMusic(int start, int end, int fadeIn, int fadeOut,
                    bool loop, int musicID, bool noResume, bool dontReset) {
        LOG_MOD_DEBUG("startMusic: musicID={}",musicID);

        if (lso::utils::offset::shouldSkipOffset(getTotalOffset())) {
            LOG_MOD_DEBUG("startMusic: skipping hook because not in a level");
            s_paddedTracks.setOriginal(0);
            FMODAudioEngine::startMusic(start, end, fadeIn, fadeOut, loop, musicID, noResume, dontReset);
            return;
        }

        int newStart = start;

        bool isPadded = s_paddedTracks.m_isPaddedNow;
        auto offset = applyOffset(start, isPadded);

        newStart=offset.adjustedTime;
        
        if(isPadded || start!=newStart){
            LOG_MOD_DEBUG("startMusic: applying offset ({} -> {}), musicID={}, padded={}", start, newStart, musicID, isPadded);
        }

        FMODAudioEngine::startMusic(
            newStart, end, fadeIn, fadeOut, loop,
            musicID, noResume, dontReset
        );
    }

    // Loads and immediately starts playback at a given time position.
    // Called by song triggers mid-level to switch music.
    // Needs to check for padded files just like queueStartMusic.
    void loadAndPlayMusic(gd::string path, unsigned int time, int musicID) {
        LOG_MOD_DEBUG("loadAndPlayMusic: path={}, musicID={}", path, musicID);

        int totalOffset = getTotalOffset();
        if (lso::utils::offset::shouldSkipOffset(totalOffset)) {
            LOG_MOD_DEBUG("loadAndPlayMusic: skipping hook because not in a level");
            s_paddedTracks.setOriginal(0);
            FMODAudioEngine::loadAndPlayMusic(path, time, musicID);
            return;
        }

        gd::string newPath = path;
        unsigned int newTime = time;
        bool isMusicPadded = false;

        // Check padded
        if (lso::config::shouldDoNegativeOffsetWorkaround(totalOffset)) {
            auto paddedResult = PaddedTrackManager::get().getPaddedResult(totalOffset, path);
            isMusicPadded = paddedResult.isPadded;
            newPath = paddedResult.resultingPath;
        }

        s_paddedTracks.setPaddedFlag(isMusicPadded, 0);

        // Apply offset
        bool isPadded = s_paddedTracks.m_isPaddedNow;
        auto offset = applyOffset(static_cast<int>(time), isPadded);
        newTime = static_cast<unsigned int>(offset.adjustedTime);
        
        if(isPadded || time!=newTime){
            LOG_MOD_DEBUG("loadAndPlayMusic: applying offset ({} -> {}), path=('{}' -> '{}'), padded={}", time, newTime, path, newPath, isPadded);
        }
        
        FMODAudioEngine::loadAndPlayMusic(
            newPath,
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

        if(lso::utils::offset::shouldSkipOffset(getTotalOffset())){
            LOG_MOD_DEBUG("triggerQueuedMusic: skipping hook because not in a level");
            FMODAudioEngine::triggerQueuedMusic(music);
            return;
        }
        // bool isPadded = s_paddedTracks.isPaddedByChannel(music.m_channelID);
        // bool isPadded = s_paddedTracks.m_isPaddedNow;
        bool isPadded = lso::utils::isFilePadded(music.m_filePath);

        auto offset = applyOffset(music.m_start, isPadded);
        if (isPadded || offset.adjustedTime != music.m_start) {
            LOG_MOD_DEBUG("triggerQueuedMusic: applying offset to m_start ({} -> {}), channel={}, padded={}",
                      music.m_start, offset.adjustedTime, music.m_channelID, isPadded);
        }
        // music.m_start = offset.adjustedTime;
        FMODAudioEngine::triggerQueuedMusic(music);
    }

    // ─── setMusicTimeMS ─────────────────────────────────────────────────────
    // Seeks music to a given time. Used by checkpoint restoration, pause, etc.

    void setMusicTimeMS(unsigned int ms, bool p1, int channel) {
        LOG_MOD_DEBUG("setMusicTimeMS: channelID={}", channel);
        int totalOffset = getTotalOffset();
        if(lso::utils::offset::shouldSkipOffset(totalOffset)){
            LOG_MOD_DEBUG("setMusicTimeMS: skipping hook because not in a level");
            FMODAudioEngine::setMusicTimeMS(ms, p1, channel);
            return;
        }
        // Flag set in last getAudioFileName call
        bool isPadded = s_paddedTracks.m_isPaddedNow;

        auto offset = applyOffset(ms, isPadded);
        if (isPadded || offset.adjustedTime != static_cast<int>(ms)) {
            LOG_MOD_DEBUG("setMusicTimeMS: {} -> {} (channel={}, padded={}, totalOffset={})", ms, offset.adjustedTime, channel, isPadded, totalOffset);
        }
        FMODAudioEngine::setMusicTimeMS(
            static_cast<unsigned int>(offset.adjustedTime), p1, channel
        );
    }
};
