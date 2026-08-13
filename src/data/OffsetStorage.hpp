#pragma once

#include <Geode/Geode.hpp>

#include "OffsetData.hpp"

using namespace geode::prelude;

/**
 * Per-level offset storage backed by mod save data.
 * Data is persisted as a JSON object: { "levelId": offset, ... }
 * inside the mod's save container.
 *
 * For editor levels (m_levelID == 0), uses EditorIDs API to get a
 * persistent unique ID so each editor level has its own offset.
 */

class OffsetStorage {
    OffsetData m_data;
public:

    static OffsetStorage &get() {
        static OffsetStorage instance;
        return instance;
    }

    /**
     * Get the stored offset (in milliseconds) for a given level.
     * Returns 0 if no offset has been set.
     */
    int getOffsetForLevel(int levelId);

    /**
     * Set the offset (in milliseconds) for a given level and persist to save data.
     */
    void setOffsetForLevel(int levelId, int offset);

    void load();
    void save();
};
