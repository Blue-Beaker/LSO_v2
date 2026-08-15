#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>

#include "OffsetButton.hpp"

using namespace geode::prelude;

class OffsetPopup final : public geode::Popup {
protected:
    TextInput* m_offsetInput = nullptr;
    int m_levelId = 0;
    GJGameLevel* m_level = nullptr;
    OffsetButton* offsetButton = nullptr;

    bool setup(GJGameLevel* level, OffsetButton* button, int currentOffset);
    void onApply(CCObject*);
    void onCancel(CCObject*);

public:
    static OffsetPopup* create(GJGameLevel* level, OffsetButton* button, int currentOffset);
};

void showOffsetPopup(GJGameLevel* level, OffsetButton* button);