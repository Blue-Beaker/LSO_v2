#include "OffsetStorage.hpp"

#include "../utils/Utils.hpp"

static auto constexpr SAVE_KEY = "level-offsets";

int OffsetStorage::getOffsetForLevel(int const levelId) {
    if (levelId==0) {
        return 0;
    }
    if (offsetData.offsets.contains(levelId)) {
        return offsetData.offsets[levelId];
    }
    return 0;
}

void OffsetStorage::setOffsetForLevel(int const levelId, int const offsetMs) {
    if (levelId==0) {
        return;
    }
    if (offsetMs==0) {
        offsetData.offsets.erase(levelId);
    }
    offsetData.offsets[levelId]=offsetMs;
    save();
}

void OffsetStorage::load() {
    auto const newData = Mod::get()->getSavedValue<OffsetData>(SAVE_KEY, offsetData);
    offsetData.offsets=newData.offsets;
    LOG_MOD_DEBUG("Loaded Offsets: {}", newData.offsets);

}
void OffsetStorage::save() const {
    Mod::get()->setSavedValue<OffsetData>(SAVE_KEY, offsetData);
}
