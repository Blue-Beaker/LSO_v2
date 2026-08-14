#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

/**
 * Manages per-level song offset - pure logic, no hooks.
 *
 * Hooks are in src/hooks/:
 *   PlayLayerHooks.cpp       - MyPlayLayer, NegativeOffsetPlayLayer
 *   FMODAudioEngineHooks.cpp - MyFMODAudioEngine
 *   GJGameLevelHooks.cpp     - NegativeOffsetGJGameLevel
 *   LevelInfoLayerHooks.cpp  - OffsetLevelInfoLayer
 *   EditLevelLayerHooks.cpp  - OffsetEditLevelLayer
 */

// Set current level, to track the offset
void setCurrentLevel(GJGameLevel* currentLevel);
GJGameLevel* getCurrentLevel();
// Set total offset for the controller.
void updateOffsets();
int getTotalOffset();
int getCurrentLevelOffset();