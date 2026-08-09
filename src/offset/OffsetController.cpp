#include "OffsetController.hpp"
#include "OffsetStorage.hpp"
#include "negative-offset-workaround/wavHelper.hpp"
#include "negative-offset-workaround/AsyncPregenerator.hpp"
#include "../utils/Utils.hpp"

// Stores the effective totalOffset (ms) for the current PlayLayer.
// Set in prepareMusic, read by getAudioFileName, queueStartMusic,
// and setMusicTimeMS hooks.
// totalOffset = original GameManager::m_timeOffset + user offset.
int s_currentTotalOffset = 0;

void setTotalOffset(int totalOffset) {
    s_currentTotalOffset = totalOffset;
}
int getTotalOffset() {
    return s_currentTotalOffset;
}

// ─── Public: start pre-generation for a level ───────────────────────────────

void startPregenerateForLevel(GJGameLevel* level) {
    if (!level) return;
    bool fixEnabled = lso::config::isNegativeOffsetFixEnabled();;
    if (!fixEnabled) return;

    auto* audio = FMODAudioEngine::sharedEngine();
    if (!audio) return;

    int userOffset = OffsetStorage::getOffsetForLevel(lso::utils::getLevelId(level));
    int originalOffset = audio->m_musicOffset;
    int totalOffset = originalOffset + userOffset;

    if (totalOffset >= 0) return;

    auto& pregen = AsyncPregenerator::get();
    if (pregen.isRunning()) {
        LOG_MOD_DEBUG("startPregenerateForLevel: generation already in progress, skipping");
        return;
    }

    auto tasks = collectPregenerateTasks(level, totalOffset);

    auto songKeys = lso::utils::getLevelSongKeys(level);

    if (!tasks.empty()) {
        pregen.generate(std::move(tasks), nullptr);
    }

    enforceCacheSizeLimit(songKeys, totalOffset);
}
