#pragma once

#include <Geode/Geode.hpp>

#include "../offset/OffsetController.hpp"

using namespace geode::prelude;

namespace lso::utils::offset{
    inline bool shouldSkipOffset(){
        // When not in a level
        if (!OffsetController::get().hasCurrentLevel()) return true;

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
