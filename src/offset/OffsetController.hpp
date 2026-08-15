#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class OffsetController final {
    int currentLevelOffset = 0;
    int currentLevelId = 0;
public:
    static OffsetController& get() {
        static OffsetController s_controller;
        return s_controller;
    }
    // Set current level, to track the offset
    void setCurrentLevel(GJGameLevel* level);
    // Set total offset for the controller.
    void updateOffsets();
    // Get offset for the current level
    [[nodiscard]] int getCurrentLevelOffset() const;

    [[nodiscard]] bool hasCurrentLevel() const;
};