#include <Geode/modify/GJBaseGameLayer.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/QueuedMusicTracker.hpp"

using namespace geode::prelude;

class $modify(GJBaseGameLayerHook, GJBaseGameLayer) {
    void processMoveActionsStep(float deltaSeconds, bool visibleFrame) {
        GJBaseGameLayer::processMoveActionsStep(deltaSeconds, visibleFrame);
        QueuedMusicTracker::get().tickFloatSeconds(deltaSeconds);
    }
};