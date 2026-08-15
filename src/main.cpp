#include "data/OffsetStorage.hpp"
using namespace geode::prelude;

/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;

// Load/Save stored offsets at startup/exiting
$on_game(ModsLoaded) {
    OffsetStorage::get().load();
}
$on_game(Exiting) {
    OffsetStorage::get().save();
}