#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include "../ui/OffsetPopup.hpp"
#include "../ui/OffsetButton.hpp"

using namespace geode::prelude;

class $modify(OffsetLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        OffsetButton* offsetBtn;
    };
    // Add the offset button to LevelInfoLayer
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;

        auto const offsetBtn = OffsetButton::create(this, menu_selector(OffsetLevelInfoLayer::onOffsetButton), EditorIDs::getID(level));
        m_fields->offsetBtn=offsetBtn;

        offsetBtn->setPosition(80, 0);

        if (auto const menu = this->getChildByID("other-menu")) {
            menu->addChild(offsetBtn);
            offsetBtn->setID("offset-button"_spr);
            menu->updateLayout();
        }

        return true;
    }

    void onOffsetButton(CCObject*) {
        showOffsetPopup(this->m_level,this->m_fields->offsetBtn);
    }
};
