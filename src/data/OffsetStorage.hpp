#pragma once

#include "OffsetData.hpp"

using namespace geode::prelude;

// The storage class for storing per-level offsets in this mod.
class OffsetStorage final {
    OffsetData offsetData;
public:

    static OffsetStorage &get() {
        static OffsetStorage instance;
        return instance;
    }

    // Get the stored offset (in milliseconds) for a given level, or 0 for no offset.
    int getOffsetForLevel(int levelId);

    // Set the offset for level (in milliseconds). offsetMs==0 removes the entry (no offset).
    void setOffsetForLevel(int levelId, int offsetMs);

    void load();
    void save() const;
};
