#include <Geode/modify/PlayLayer.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/OffsetTracker.hpp"
#include "../offset/QueuedMusicTracker.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;
        if (m_level) {
            OffsetController::get().setCurrentLevel(m_level);
            OffsetController::get().updateOffsets();
        }
        return true;
    }

    void prepareMusic(bool dontWait) {
        if (auto* audio = FMODAudioEngine::sharedEngine()) {
            LOG_MOD_DEBUG("BEFORE prepareMusic: m_musicOffset={}, dontWait={}", audio->m_musicOffset, dontWait);
        }

        PlayLayer::prepareMusic(dontWait);
        if (auto* audio = FMODAudioEngine::sharedEngine()) {
            LOG_MOD_DEBUG("AFTER prepareMusic: m_musicOffset={}, dontWait={}", audio->m_musicOffset, dontWait);
        }
    }

    // void startMusic() {
    //     LOG_MOD_DEBUG("PlayLayer::startMusic")
    //     PlayLayer::startMusic();
    // }
    //


// TODO(FIXTHIS): When set offset to negative, Trigger a song trigger (not prep) and pause before the song actually plays, then resume, the music will start immediately, ignoring our queue.
    void resume() {
        OffsetController::get().setCurrentLevel(m_level);
        OffsetController::get().updateOffsets();
        LOG_MOD_DEBUG("PlayLayer::resume");
        PlayLayer::resume();
        QueuedMusicTracker::get().pauseAll();
    }
    void onQuit(){
        PlayLayer::onQuit();
        LOG_MOD_DEBUG("PlayLayer::onQuit");
        OffsetController::get().setCurrentLevel(nullptr);
        OffsetController::get().updateOffsets();
        OffsetTracker::get().clear();
        QueuedMusicTracker::get().clear();
    }
    //
    // void postUpdate(float deltaSeconds) {
    //     PlayLayer::postUpdate(deltaSeconds);
    //     if (PlayLayer::isGameplayActive()) {
    //         QueuedMusicTracker::get().tickFloatSeconds(deltaSeconds);
    //     }
    // }
};