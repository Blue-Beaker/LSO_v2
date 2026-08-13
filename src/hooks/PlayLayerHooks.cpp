#include <Geode/modify/PlayLayer.hpp>

#include "../offset/OffsetController.hpp"
#include "../data/OffsetStorage.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

// ─── Hook: PlayLayer ────────────────────────────────────────────────────────

class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;
        if (m_level) {
            setCurrentLevel(m_level);
        }
        return true;
    }

    void prepareMusic(bool dontWait) {
        if (auto* audio = FMODAudioEngine::sharedEngine()) {
            LOG_MOD_DEBUG("BEFORE prepareMusic: m_musicOffset={}", audio->m_musicOffset);
        }
        if (m_level) {
            int userOffset = OffsetStorage::get().getOffsetForLevel(lso::utils::getLevelId(m_level));
            int originalOffset = FMODAudioEngine::sharedEngine()->m_musicOffset;
            int totalOffset = originalOffset + userOffset;

            LOG_MOD_DEBUG("prepareMusic: level={}, userOffset={}, originalOffset={}, totalOffset={}", lso::utils::getLevelId(m_level), userOffset, originalOffset, totalOffset);

            setTotalOffset(totalOffset);
            setCurrentLevel(m_level);

            if (lso::config::shouldDoNegativeOffsetWorkaround(totalOffset)) {
            }
        }

        PlayLayer::prepareMusic(dontWait);
        if (auto* audio = FMODAudioEngine::sharedEngine()) {
            LOG_MOD_DEBUG("AFTER prepareMusic: m_musicOffset={}", audio->m_musicOffset);
        }
    }

    void onExit(){
        PlayLayer::onExit();
        int originalOffset = FMODAudioEngine::sharedEngine()->m_musicOffset;
        setTotalOffset(originalOffset);
        setCurrentLevel(nullptr);
    }
};