#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

#include "../ui/OffsetPopup.hpp"
#include "../ui/OffsetButton.hpp"

using namespace geode::prelude;

class $modify(OffsetEditLevelLayer, EditLevelLayer) {
    struct Fields {
        OffsetButton* offsetBtn = nullptr;
    };
    // Add the offset button to EditLevelLayer
    // Warning: the offset only works when "playing" the level, and does not work in the editor.
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

        auto offsetBtn = OffsetButton::create(this, menu_selector(OffsetEditLevelLayer::onOffsetButton), EditorIDs::getID(level));
        m_fields->offsetBtn=offsetBtn;

        auto menu = this->getChildByID("info-button-menu");
        if (menu) {
            menu->addChild(offsetBtn);
            offsetBtn->setID("offset-button"_spr);
            offsetBtn->setPosition(80, 0);
            menu->updateLayout();
        }

        return true;
    }

    void onOffsetButton(CCObject*) {
        showOffsetPopup(this->m_level,this->m_fields->offsetBtn);
    }
};
