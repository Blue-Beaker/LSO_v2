#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// A wrapper around the offset icon button that displays an offset value and the value can be updated.
class OffsetButton final : public CCMenuItemSpriteExtra {
protected:
    int m_levelId = 0;
    CCLabelBMFont* m_offsetLabel = nullptr;

    bool init(CCObject* target, SEL_MenuHandler selector, int levelId);

public:
    static OffsetButton* create(CCObject* target, SEL_MenuHandler selector, int levelId);

    // Convenience method to update offset label from the storage
    void updateOffset();

    // Set the offset value and update the label immediately.
    void setOffset(int offset);
};
