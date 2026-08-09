#include <Geode/modify/FMODAudioEngine.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/OffsetCalculator.hpp"
#include "../offset/PaddedTrackTracker.hpp"
#include "../offset/negative-offset-workaround/CacheStorage.hpp"
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
        // When not in a level or totalOffset==0, don't apply any offset or redirect to padded files.
        if (lso::utils::offset::shouldSkipOffset(getTotalOffset())) {
            s_paddedTracks.setOriginal(musicID, channelID);
            FMODAudioEngine::queueStartMusic(
                path, pitch, unknown, volume, loop, start, end,
                fadeIn, fadeOut, musicID, p10, channelID, noPrepare, dontReset
            );
            return;
        }
        
        // The new modified values to pass to the original function. We will modify these as needed.
        gd::string newPath = path;
        int newStart = start;
        int newEnd = end;

        // When in a level, apply offset and redirect to padded files if necessary.

        int totalOffset = getTotalOffset();

        bool fixEnabled = lso::config::isNegativeOffsetFixEnabled();

        // bool isMusicPadded = false;
        bool isMusicPadded = s_paddedTracks.m_isPaddedNow;

        // Negative offset with fix enabled: redirect to padded file
        if (lso::config::shouldDoNegativeOffsetWorkaround(totalOffset)) {
            // Already a padded file - set padded state and apply remainder
            if (lso::utils::isFilePadded(path)) {
                isMusicPadded = true;
            }
            // Not a padded file - Redirect to padded file if it exists, otherwise fallback to original.
            int songKey = getSongID(musicID, path);
            auto paddedPath = getPaddedPath(songKey, totalOffset,path);

            std::error_code ec;
            if(std::filesystem::exists(paddedPath, ec)){
                isMusicPadded = true;
                newPath = paddedPath.string();
            };
        }
        
        s_paddedTracks.setPaddedFlag(isMusicPadded, musicID, channelID);

        // Because we can't distinguish between queued music that was already prepared (noPrepare=false) and queued music that is being prepared now (noPrepare=true), we only apply the offset to the start time when noPrepare=true. This ensures that we don't double-apply the offset in the case where the music is already prepared.
        if (totalOffset != 0 && noPrepare) {
            auto offset = applyOffset(start, isMusicPadded);
            newStart = offset.adjustedTime;
        }
        
        if(start!=newStart || isMusicPadded){
            LOG_MOD_DEBUG("queueStartMusic: applying offset {} to start ({} -> {}), path=('{}' -> '{}'), musicID={}, padded={}", totalOffset, start, newStart, path, newPath, musicID, isMusicPadded);
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

        if (lso::utils::offset::shouldSkipOffset(getTotalOffset())) {
            s_paddedTracks.setOriginal(musicID, 0);
            FMODAudioEngine::startMusic(start, end, fadeIn, fadeOut, loop, musicID, noResume, dontReset);
            return;
        }

        int newStart = start;

        bool isPadded = s_paddedTracks.isPaddedByMusicID(musicID);
        auto offset = applyOffset(start, isPadded);

        newStart=offset.adjustedTime;
        
        if(isPadded || start!=newStart){
            LOG_MOD_DEBUG("startMusic: applying offset ({} -> {}), musicID={}, padded={}",
                        start, newStart, musicID, isPadded);
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

        int totalOffset = getTotalOffset();
        if (lso::utils::offset::shouldSkipOffset(totalOffset)) {
            s_paddedTracks.setOriginal(musicID, 0);
            FMODAudioEngine::loadAndPlayMusic(path, time, musicID);
            return;
        }

        gd::string newPath = path;
        unsigned int newTime = time;
        bool isMusicPadded = false;

        // Check padded
        if (lso::config::shouldDoNegativeOffsetWorkaround(totalOffset)) {
            if (lso::utils::isFilePadded(path)) {
                isMusicPadded = true;
            }
            int songKey = getSongID(musicID, path);
            auto paddedPath = getPaddedPath(songKey, totalOffset, path);
            std::error_code ec;

            if (std::filesystem::exists(paddedPath, ec)) {
                isMusicPadded = true;
                newPath = paddedPath.string();
            }
        }

        s_paddedTracks.setPaddedFlag(isMusicPadded, musicID, 0);

        // Apply offset
        bool isPadded = s_paddedTracks.isPaddedByMusicID(musicID);
        auto offset = applyOffset(static_cast<int>(time), isPadded);
        newTime = static_cast<unsigned int>(offset.adjustedTime);
        
        if(isPadded || time!=newTime){
            LOG_MOD_DEBUG("loadAndPlayMusic: applying offset ({} -> {}), path=('{}' -> '{}'), padded={}",
                        time, newTime, path, newPath, isPadded);
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

        if(lso::utils::offset::shouldSkipOffset(getTotalOffset())){
            FMODAudioEngine::triggerQueuedMusic(music);
            return;
        }

        // bool isPadded = s_paddedTracks.isPaddedByChannel(music.m_channelID);
        bool isPadded = s_paddedTracks.m_isPaddedNow;
        auto offset = applyOffset(music.m_start, isPadded);
        if (isPadded || offset.adjustedTime != music.m_start) {
            LOG_MOD_DEBUG("triggerQueuedMusic: applying offset to m_start ({} -> {}), channel={}, padded={}",
                      music.m_start, offset.adjustedTime, music.m_channelID, isPadded);
        }
        music.m_start = offset.adjustedTime;
        FMODAudioEngine::triggerQueuedMusic(music);
    }

    // ─── setMusicTimeMS ─────────────────────────────────────────────────────
    // Seeks music to a given time. Used by checkpoint restoration, pause, etc.

    void setMusicTimeMS(unsigned int ms, bool p1, int channel) {
        int totalOffset = getTotalOffset();
        if(lso::utils::offset::shouldSkipOffset(totalOffset)){
            FMODAudioEngine::setMusicTimeMS(ms, p1, channel);
            return;
        }

        // bool isPadded = s_paddedTracks.isPaddedByChannel(channel);
        bool isPadded = s_paddedTracks.m_isPaddedNow;

        // If channel lookup failed, check if any song of the current level
        // is using a padded file (via musicID tracking from getAudioFileName).
        if (!isPadded && totalOffset < 0) {
            geode::log::warn("setMusicTimeMS: channel {} not found in padded tracks, checking current level songs", channel);
            if (auto* pl = PlayLayer::get()) {
                if (pl->m_level) {
                    for (int key : lso::utils::getLevelSongKeys(pl->m_level)) {
                        if (s_paddedTracks.isPaddedByMusicID(key)) {
                            isPadded = true;
                            s_paddedTracks.setPadded(key, channel);
                            break;
                        }
                    }
                }
            }
        }

        auto offset = applyOffset(ms, isPadded);
        if (isPadded || offset.adjustedTime != static_cast<int>(ms)) {
            LOG_MOD_DEBUG("setMusicTimeMS: {} -> {} (channel={}, padded={}, totalOffset={})",
                      ms, offset.adjustedTime, channel, isPadded, totalOffset);
        }
        FMODAudioEngine::setMusicTimeMS(
            static_cast<unsigned int>(offset.adjustedTime), p1, channel
        );
    }
};
