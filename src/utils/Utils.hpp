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
}

#ifdef RELWITHDEBINFO_BUILD
#define LOG_MOD_DEBUG(...) {geode::log::info(__VA_ARGS__);};
#endif
#ifndef LOG_MOD_DEBUG
#define LOG_MOD_DEBUG(...) {};
#endif