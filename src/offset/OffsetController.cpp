#include "OffsetController.hpp"

#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "../data/OffsetStorage.hpp"
#include "../utils/Utils.hpp"

// Set current level, to track the offset
// Set to nullptr to clear the offset
void OffsetController::setCurrentLevel(GJGameLevel* level){
    if (level!=nullptr) {
        currentLevelId=EditorIDs::getID(level);
    }else {
        currentLevelId=0;
    }
}

void OffsetController::updateOffsets() {
    int originalOffset = FMODAudioEngine::sharedEngine()->m_musicOffset;
    int levelOffset = OffsetStorage::get().getOffsetForLevel(currentLevelId);
    currentLevelOffset = levelOffset;

    LOG_MOD_DEBUG("updateOffsets: level={}, userOffset={}, originalOffset={}", currentLevelId, levelOffset, originalOffset);
}

int OffsetController::getCurrentLevelOffset() const {
    return currentLevelOffset;
}

bool OffsetController::hasCurrentLevel() const {
    return currentLevelId != 0;
}
