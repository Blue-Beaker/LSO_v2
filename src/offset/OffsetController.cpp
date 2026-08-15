#include "OffsetController.hpp"
#include "../data/OffsetStorage.hpp"
#include "../utils/Utils.hpp"

int s_currentTotalOffset = 0;
int s_currentLevelOffset = 0;
GJGameLevel* s_currentLevel = nullptr;
int s_currentLevelId = 0;

// Set current level, to track the offset
// Set to nullptr to clear the offset
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
