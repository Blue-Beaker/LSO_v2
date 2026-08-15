#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace lso::utils{
    // Resolve the effective level ID for a GJGameLevel.
    //
    // For published levels (m_levelID != 0), returns m_levelID directly.
    // For editor levels (m_levelID == 0), uses the EditorIDs API to get a
    // persistent unique ID.
    //
    // @param level The level to resolve. Returns 0 if null.
    int getLevelId(GJGameLevel* level);
}

namespace lso::utils::offset{
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
    inline bool isDebugLoggingEnabled() {
        return Mod::get()->getSettingValue<bool>("debug-logging");
    }
}

#define LOG_MOD_DEBUG(...) if (lso::config::isDebugLoggingEnabled()){geode::log::info(__VA_ARGS__);};
