#include <Geode/modify/MusicDownloadManager.hpp>
#include "../offset/negative-offset-workaround/PaddedTrackManager.hpp"
#include "../offset/OffsetController.hpp"

using namespace geode::prelude;

class $modify(MusicDownloadManagerHooks, MusicDownloadManager) {
    // Set priority to late to set path after Jukeox replacing NONG
    static void onModify(auto& self) {
        (void)self.setHookPriorityPost(
            "MusicDownloadManager::pathForSong",
            Priority::Late
        );
    }

    gd::string pathForSong(int id) {
        gd::string original = MusicDownloadManager::pathForSong(id);
        int totalOffset = getTotalOffset();
        if(!lso::config::shouldDoNegativeOffsetWorkaround(totalOffset)){
            return original;
        }
        auto result = PaddedTrackManager::get().getPaddedResult(id,totalOffset,original);
        LOG_MOD_DEBUG("MusicDownloadManagerHooks::pathForSong: redirecting path '{}' to '{}'",original,result.resultingPath);
        return result.resultingPath;
    }
};