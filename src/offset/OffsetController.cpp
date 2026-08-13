#include "OffsetController.hpp"
#include "OffsetStorage.hpp"
#include "../utils/Utils.hpp"

// Stores the effective totalOffset (ms) for the current PlayLayer.
// Set in prepareMusic, read by getAudioFileName, queueStartMusic,
// and setMusicTimeMS hooks.
// totalOffset = original GameManager::m_timeOffset + user offset.
int s_currentTotalOffset = 0;
GJGameLevel* s_currentLevel = nullptr;

void setCurrentLevel(GJGameLevel* currentLevel){
    s_currentLevel=currentLevel;
}
GJGameLevel* getCurrentLevel(){
    return s_currentLevel;
}

void setTotalOffset(int totalOffset) {
    s_currentTotalOffset = totalOffset;
}
int getTotalOffset() {
    int originalOffset = FMODAudioEngine::sharedEngine()->m_musicOffset;
    if(s_currentLevel==nullptr){
        return originalOffset;
    }
    int levelOffset = OffsetStorage::getOffsetForLevel(lso::utils::getLevelId(s_currentLevel));
    return originalOffset + levelOffset;
}

// ─── Public: start pre-generation for a level ───────────────────────────────

void startPregenerateForLevel(GJGameLevel* level) {
}
