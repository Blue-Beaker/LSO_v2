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

    std::vector<int> getLevelSongKeys(GJGameLevel* level) {
        std::vector<int> keys;
        if (!level) return keys;

        int mainKey = (level->m_songID != 0) ? level->m_songID : (-level->m_audioTrack - 1);
        keys.push_back(mainKey);

        if (!level->m_songIDs.empty()) {
            gd::string ids(level->m_songIDs);
            auto idList = geode::utils::string::split(ids, ",");
            for (const auto& idStr : idList) {
                auto key = geode::utils::numFromString<int>(geode::utils::string::filter(idStr,"0123456789"));
                if (key) keys.push_back(key.unwrap());
            }
        }

        return keys;
    }

    gd::string getFileName(const gd::string audioPath){
        std::filesystem::path p(audioPath);
        return p.filename().string();
    }

    gd::string getFileNameWithoutExtension(const gd::string audioPath){
        std::filesystem::path p(audioPath);
        return p.stem().string();
    }
}