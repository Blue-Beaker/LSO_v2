#include <Geode/modify/GJGameLevel.hpp>

#include "../offset/negative-offset-workaround/CacheStorage.hpp"
#include "../offset/negative-offset-workaround/AsyncPregenerator.hpp"
#include "../offset/PaddedTrackTracker.hpp"
#include "../offset/OffsetController.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

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
