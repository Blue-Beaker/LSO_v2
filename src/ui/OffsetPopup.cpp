#include "OffsetPopup.hpp"

#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "../data/OffsetStorage.hpp"

bool OffsetPopup::setup(GJGameLevel* level, OffsetButton* button, int currentOffset) {
    if (!level) return false;
    this->offsetButton=button;

    m_levelId = EditorIDs::getID(level);
    m_level = level;
    this->setTitle("Level Song Offset");

    // Input label
    auto const label = CCLabelBMFont::create("Offset (ms):", "bigFont.fnt");
    label->setScale(0.6f);
    m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(0, 25));

    // Text input
    m_offsetInput = TextInput::create(200.f, "0", "bigFont.fnt");
    m_offsetInput->setString(fmt::format("{}", currentOffset));
    m_offsetInput->setCommonFilter(CommonFilter::Int);
    m_offsetInput->setMaxCharCount(8);
    m_mainLayer->addChildAtPosition(m_offsetInput, Anchor::Center, ccp(0, -5));

    // Apply button
    auto const okBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply"),
        this,
        menu_selector(OffsetPopup::onApply)
    );

    // Cancel button
    // auto cancelBtn = CCMenuItemSpriteExtra::create(
    //     ButtonSprite::create("Cancel"),
    //     this,
    //     menu_selector(OffsetPopup::onCancel)
    // );

    auto menu = CCMenu::create();
    menu->addChild(okBtn);
    // menu->addChild(cancelBtn);
    menu->alignItemsHorizontallyWithPadding(20);
    m_mainLayer->addChildAtPosition(menu, Anchor::Center, ccp(0, -50));

    return true;
}

void OffsetPopup::onApply(CCObject*) {
    int offset = 0;

    if (auto const result = numFromString<int>(m_offsetInput->getString()); result.isOk()) {
        offset = result.asOk().unwrap();
    }
    OffsetStorage::get().setOffsetForLevel(m_levelId, offset);
    log::debug("Set offset for level {} (resolved) to {}ms", m_levelId, offset);

    Notification::create(
        fmt::format("Offset set to {}ms for level {}", offset, m_levelId),
        NotificationIcon::Success
    )->show();

    if(this->offsetButton!=nullptr){
        this->offsetButton->setOffset(offset);
    }

    this->onClose(nullptr);
}

void OffsetPopup::onCancel(CCObject*) {
    this->onClose(nullptr);
}

OffsetPopup* OffsetPopup::create(GJGameLevel* level, OffsetButton* button, int const currentOffset) {
    auto const ret = new OffsetPopup();
    if (ret->init(280.f, 180.f)) {
        ret->setup(level, button, currentOffset);
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

// Show the offset popup for a level, reading stored offset from the storage.
void showOffsetPopup(GJGameLevel* level, OffsetButton* button) {
    if (!level) return;
    int const currentOffset = OffsetStorage::get().getOffsetForLevel(EditorIDs::getID(level));
    auto const popup = OffsetPopup::create(level, button, currentOffset);
    popup->show();
}