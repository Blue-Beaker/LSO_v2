#include <Geode/modify/GJGameLevel.hpp>

#include "../offset/negative-offset-workaround/CacheStorage.hpp"
#include "../offset/negative-offset-workaround/AsyncPregenerator.hpp"
#include "../offset/PaddedTrackTracker.hpp"
#include "../offset/OffsetController.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

// ─── Hook: GJGameLevel::getAudioFileName ─────────────────────────────────────
// Compute the padded file path on the fly from the original audio path.
// If the total offset is negative and the padded file exists, return it;
// otherwise return the original.
//
// IMPORTANT: Only redirect when totalOffset < 0. If a padded file from a
// previous session still exists but the user has since set a positive (or
// zero) offset, we must NOT use the padded file.

class $modify(NegativeOffsetGJGameLevel, GJGameLevel) {
    gd::string getAudioFileName() {
        auto original = GJGameLevel::getAudioFileName();
        if (original.empty()) return original;
        int totalOffset = getTotalOffset();
        // Only redirect to padded file when should
        bool shouldRedirect = lso::config::shouldDoNegativeOffsetWorkaround(totalOffset);
        if (!shouldRedirect) return original;

        auto* fileUtils = CCFileUtils::sharedFileUtils();
        gd::string fullPath = fileUtils->fullPathForFilename(original.c_str(), false);
        if (fullPath.empty()) return original;
        auto srcPath = std::filesystem::path(fullPath);
        int songKey = getSongKey(this);

        auto paddedPath = getPaddedPath(songKey, totalOffset, fullPath);
        std::error_code ec;
        if (std::filesystem::exists(paddedPath, ec)) {
            log::debug("Using padded audio: {}", paddedPath.string());
            // Set padded state via musicID so that hooks like setMusicTimeMS
            // can detect this track uses a padded file, even if queueStartMusic
            // hook doesn't get called.
            s_paddedTracks.setPaddedByMusicID(songKey);
            return gd::string(paddedPath.string());
        }

        return original;
    }
};
