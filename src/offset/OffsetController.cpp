#include "OffsetController.hpp"
#include "../data/OffsetStorage.hpp"
#include "../utils/Utils.hpp"

// Stores the effective totalOffset (ms) for the current PlayLayer.
// Set in prepareMusic, read by getAudioFileName, queueStartMusic,
// and setMusicTimeMS hooks.
// totalOffset = original GameManager::m_timeOffset + user offset.
int s_currentTotalOffset = 0;
int s_currentLevelOffset = 0;

GJGameLevel* s_currentLevel = nullptr;
int s_currentLevelId = 0;

void setCurrentLevel(GJGameLevel* currentLevel){
    s_currentLevel=currentLevel;
    if (currentLevel!=nullptr) {
        s_currentLevelId=lso::utils::getLevelId(s_currentLevel);
    }else {
        s_currentLevelId=0;
    }
}
GJGameLevel* getCurrentLevel(){
    return s_currentLevel;
}

void updateOffsets() {
    int originalOffset = FMODAudioEngine::sharedEngine()->m_musicOffset;
    int levelOffset = OffsetStorage::get().getOffsetForLevel(s_currentLevelId);
    int totalOffset = levelOffset + originalOffset;

    s_currentTotalOffset = totalOffset;
    s_currentLevelOffset = levelOffset;

    LOG_MOD_DEBUG("updateOffsets: level={}, userOffset={}, originalOffset={}, totalOffset={}", s_currentLevelId, levelOffset, originalOffset, totalOffset);
}
int getTotalOffset() {
    return s_currentTotalOffset;
}
int getCurrentLevelOffset() {
    return s_currentLevelOffset;
}
