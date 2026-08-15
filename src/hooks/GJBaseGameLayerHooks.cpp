#include <Geode/modify/GJBaseGameLayer.hpp>

#include "../offset/OffsetController.hpp"
#include "../data/OffsetStorage.hpp"
#include "../offset/OffsetTracker.hpp"
#include "../offset/QueuedMusicTracker.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

class $modify(GJBaseGameLayerHook, GJBaseGameLayer) {
    void processMoveActionsStep(float deltaSeconds, bool visibleFrame) {
        GJBaseGameLayer::processMoveActionsStep(deltaSeconds, visibleFrame);
        QueuedMusicTracker::get().tickFloatSeconds(deltaSeconds);
    }
};