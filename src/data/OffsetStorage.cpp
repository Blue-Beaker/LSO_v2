#include "OffsetStorage.hpp"

#include "../utils/Utils.hpp"

static constexpr auto SAVE_KEY = "level-offsets";

int OffsetStorage::getOffsetForLevel(int levelId) {
    if (levelId==0) {
        return 0;
    }
    if (m_data.m_offsets.contains(levelId)) {
        return m_data.m_offsets[levelId];
    }
    return 0;
}

void OffsetStorage::setOffsetForLevel(int levelId, int offset) {
    if (levelId==0) {
        return;
    }
    if (offset==0) {
        m_data.m_offsets.erase(levelId);
    }
    m_data.m_offsets[levelId]=offset;
    save();
}

void OffsetStorage::load() {
    const auto newData = Mod::get()->getSavedValue<OffsetData>(SAVE_KEY, m_data);
    m_data.m_offsets=newData.m_offsets;
    LOG_MOD_DEBUG("Loaded Offsets: {}", newData.m_offsets);

}
void OffsetStorage::save() {
    Mod::get()->setSavedValue<OffsetData>(SAVE_KEY, m_data);
}
