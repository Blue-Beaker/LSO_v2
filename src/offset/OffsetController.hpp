#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// Set current level, to track the offset
void setCurrentLevel(GJGameLevel* currentLevel);
GJGameLevel* getCurrentLevel();
// Set total offset for the controller.
void updateOffsets();
int getTotalOffset();
int getCurrentLevelOffset();