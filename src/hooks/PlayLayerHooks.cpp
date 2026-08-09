#include <Geode/modify/PlayLayer.hpp>

#include "../offset/OffsetController.hpp"
#include "../offset/OffsetStorage.hpp"
#include "../offset/negative-offset-workaround/wavHelper.hpp"
#include "../offset/negative-offset-workaround/AsyncPregenerator.hpp"
#include "../utils/Utils.hpp"
#include "../offset/negative-offset-workaround/CacheStorage.hpp"

using namespace geode::prelude;

// ─── Hook: PlayLayer ────────────────────────────────────────────────────────

class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        // Start async pre-generation of padded audio files in the background.
        if (m_level) {
            startPregenerateForLevel(m_level);
        }

        return true;
    }

    void prepareMusic(bool dontWait) {
        if (auto* audio = FMODAudioEngine::sharedEngine()) {
            LOG_MOD_DEBUG("BEFORE prepareMusic: m_musicOffset={}", audio->m_musicOffset);
        }
        if (m_level) {
            int userOffset = OffsetStorage::getOffsetForLevel(lso::utils::getLevelId(m_level));
            int originalOffset = FMODAudioEngine::sharedEngine()->m_musicOffset;
            int totalOffset = originalOffset + userOffset;

            LOG_MOD_DEBUG("prepareMusic: level={}, userOffset={}, originalOffset={}, totalOffset={}", lso::utils::getLevelId(m_level), userOffset, originalOffset, totalOffset);

            setTotalOffset(totalOffset);

            if (lso::config::shouldDoNegativeOffsetWorkaround(totalOffset)) {
                auto& pregen = AsyncPregenerator::get();
                if (pregen.isRunning()) {
                    LOG_MOD_DEBUG("prepareMusic: waiting for pre-generation...");
                    pregen.waitAll();
                    LOG_MOD_DEBUG("prepareMusic: pre-generation done");
                }
            }
        }

        PlayLayer::prepareMusic(dontWait);
        if (auto* audio = FMODAudioEngine::sharedEngine()) {
            LOG_MOD_DEBUG("AFTER prepareMusic: m_musicOffset={}", audio->m_musicOffset);
        }
    }
};