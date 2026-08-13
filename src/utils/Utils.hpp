#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace lso::utils{
    // ─── Level ID resolution ────────────────────────────────────────────────────

    // Resolve the effective level ID for a GJGameLevel.
    //
    // For published levels (m_levelID != 0), returns m_levelID directly.
    // For editor levels (m_levelID == 0), uses the EditorIDs API to get a
    // persistent unique ID.
    //
    // @param level The level to resolve. Returns 0 if null.
    int getLevelId(GJGameLevel* level);

    // ─── Song key collection ────────────────────────────────────────────────────

    // Collect all song keys for a level into a vector.
    // Includes the main song key (m_songID or -m_audioTrack - 1) plus any
    // additional song IDs from m_songIDs (comma-separated string).
    //
    // @param level The level to collect song keys from.
    // @return A vector of all song keys for the level.
    std::vector<int> getLevelSongKeys(GJGameLevel* level);

    boolean isFilePadded(const gd::string audioFilename);

    gd::string getFileName(const gd::string audioFilename);

    gd::string getFileNameWithoutExtension(const gd::string audioFilename);
}

namespace lso::utils::offset{
    // Calculates the padded length (in milliseconds) for a given total offset.
    // ALL padded length calculations should use this function to ensure consistency.
    inline int calculatePaddedLength(int totalOffset) {
        if (totalOffset >= 0) return 0;
        return ((-totalOffset + 999) / 1000) * 1000;
    }
    inline bool shouldSkipOffset(){
        // When not in a level
        auto* pl = PlayLayer::get();
        if (!pl || !pl->m_level) return true;

        return false;
    }
}

namespace lso::config{
    inline bool isNegativeOffsetFixEnabled() {
        return Mod::get()->getSettingValue<bool>("negative-offset-fix");
    }
    inline bool shouldDoNegativeOffsetWorkaround(int totalOffset) {
        return (totalOffset < 0 && isNegativeOffsetFixEnabled());
    }
    inline bool isDebugLoggingEnabled() {
        return Mod::get()->getSettingValue<bool>("debug-logging");
    }
}

#define LOG_MOD_DEBUG(...) if (lso::config::isDebugLoggingEnabled()){geode::log::info(__VA_ARGS__);};
