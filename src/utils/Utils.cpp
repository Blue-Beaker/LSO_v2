#include "Utils.hpp"
#include "../ui/OffsetPopup.hpp"

#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

namespace lso::utils{
    int getLevelId(GJGameLevel* level) {
        if (!level) return 0;
        if (level->m_levelID != 0) return level->m_levelID;
        return EditorIDs::getID(level);
    }
}
