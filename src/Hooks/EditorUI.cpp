#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditButtonBar.hpp>

#include "../Version.hpp"

using namespace geode::prelude;

class $modify (RetroEditorUI, EditorUI)
{
    struct Fields
    {
        std::vector<EditButtonBar*> bars;
        CCMenu* tabsMenu;
        std::vector<CCMenuItemToggler*> tabs;
        Ref<CCArray> objs;
        int selectedTab = 0;
        bool initialized = false;
    };

    bool init(LevelEditorLayer* editorLayer)
    {
        if (!EditorUI::init(editorLayer))
            return false;

        if (m_fields->initialized)
            return true;

        int rows = 6;
        int columns = 2;

        std::string version = VersionUtils::getVersionSimulating();
        if (version.empty()) version = "1.9";

        auto tabs = VersionUtils::getTabs(version);
        m_fields->tabs = tabs;

        auto tabsMenu = CCMenu::create();
        if (!tabsMenu)
        {
            m_fields->initialized = true;
            return true;
        }

        tabsMenu->setLayout(RowLayout::create()->setGap(2.0f)->setCrossAxisLineAlignment(AxisAlignment::Start));
        
        if (m_tabsMenu)
            tabsMenu->setPosition(m_tabsMenu->getPosition());
        else
            tabsMenu->setPosition(ccp(0, 0));
        
        tabsMenu->setZOrder(69);
        tabsMenu->setID("tabs-menu"_spr);
        m_fields->tabsMenu = tabsMenu;

        int i = 0;
        for (auto tab : tabs)
        {
            if (!tab) continue;
            tabsMenu->addChild(tab);
            auto toggler = typeinfo_cast<CCMenuItemToggler*>(tab);
            if (!toggler)
                continue;
            toggler->setTarget(this, menu_selector(RetroEditorUI::onChangeTab));
            toggler->setTag(i);
            i++;
        }

        tabsMenu->updateLayout();

        auto objs = CCArray::create();
        if (!objs)
        {
            m_fields->initialized = true;
            return true;
        }
        m_fields->objs = objs;

        for (size_t i = 0; i < tabs.size(); i++)
        {
            auto objs2 = VersionUtils::getObjectsForVersion(version, rows, columns, this, i);
            if (!objs2) continue;

            for (auto obj : CCArrayExt<CCMenuItemSpriteExtra*>(objs2))
            {
                if (obj) objs->addObject(obj);
            }

            auto editBar = EditButtonBar::create(objs2, ccp(CCDirector::get()->getWinSize().width / 2 - 5, 86), 69, false, rows, columns);
            if (!editBar) continue;
            editBar->setZOrder(10);
            editBar->setID("edit-bar"_spr);
            m_fields->bars.push_back(editBar);
            this->addChild(editBar, 5 + i);
        }

        selectTab(0);
        this->addChild(tabsMenu);
        
        m_fields->initialized = true;
        return true;
    }

    void setupCreateMenu()
    {
        EditorUI::setupCreateMenu();
        if (auto tabs = m_tabsMenu)
            tabs->setScale(0);
        for (auto tab : CCArrayExt<CCNode*>(m_createButtonBars))
        {
            if (tab) tab->setScale(0);
        }
        Loader::get()->queueInMainThread([this]
        {
            for (auto tab : CCArrayExt<CCNode*>(m_createButtonBars))
            {
                if (tab) tab->setScale(0);
            }
        });
    }

    void selectTab(int index)
    {
        if (!m_fields->initialized)
            return;
        if (m_fields->tabs.size() <= 1)
            return;
        m_fields->selectedTab = index;
        for (size_t i = 0; i < m_fields->tabs.size(); i++)
        {
            auto tab = m_fields->tabs[i];
            if (!tab) continue;
            tab->toggle(i == index);
            tab->setEnabled(i != index);
            if (tab->m_offButton)
            {
                tab->m_offButton->setScale(1);
                tab->m_offButton->stopAllActions();
            }
            if (tab->m_onButton)
            {
                tab->m_onButton->setScale(1);
                tab->m_onButton->stopAllActions();
            }
        }
        updateCustomTabs();
    }

    void onChangeTab(CCObject* sender)
    {
        if (!sender) return;
        selectTab(sender->getTag());
        updateCustomTabs();
    }

    void updateCustomTabs()
    {
        if (!m_fields->initialized)
            return;
        if (!m_fields->tabsMenu)
            return;
        for (size_t i = 0; i < m_fields->bars.size(); i++)
        {
            if (m_fields->bars[i])
                m_fields->bars[i]->setVisible(m_tabsMenu && m_tabsMenu->isVisible() && i == m_fields->selectedTab);
        }
        if (m_fields->tabsMenu)
            m_fields->tabsMenu->setVisible(m_tabsMenu && m_tabsMenu->isVisible());
    }

    void resetUI()
    {
        EditorUI::resetUI();
        updateCustomTabs();
    }

    void updateCreateMenu(bool p0)
    {
        EditorUI::updateCreateMenu(p0);
        if (!m_fields->initialized)
            return;
        for (auto btn : CCArrayExt<CCNode*>(m_fields->objs))
        {
            if (!btn) continue;
            auto tbtn = typeinfo_cast<CreateMenuItem*>(btn);
            if (!tbtn) continue;
            
            auto btnSpr = typeinfo_cast<ButtonSprite*>(tbtn->getNormalImage());
            if (!btnSpr) continue;
            
            auto colourSpr = typeinfo_cast<CCSprite*>(tbtn->getChildByID("colour"_spr));
            if (!colourSpr) continue;
            
            auto col3 = colourSpr->getColor();
            auto col = btn->getTag() == m_selectedObjectIndex ? ccc3(127, 127, 127) : ccWHITE;
            
            if (btnSpr->m_subBGSprite)
                btnSpr->m_subBGSprite->setColor(col);
        }
    }
};