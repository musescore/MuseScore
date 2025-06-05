/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "toolbaritem.h"

#include <QVariantMap>

#include "types/translatablestring.h"
#include "shortcuts/shortcutstypes.h"

using namespace muse::uicomponents;
using namespace muse::ui;

ToolBarItem::ToolBarItem(QObject* parent)
    : QObject(parent)
{
}

ToolBarItem::ToolBarItem(const UiAction& action, ToolBarItemType::Type type, QObject* parent)
    : QObject(parent)
{
    m_id = QString::fromStdString(action.code);
    m_action = action;
    m_type = type;
}

void ToolBarItem::activate()
{
    dispatcher()->dispatch(action().code, args());
}

void ToolBarItem::handleMenuItem(const QString& menuId)
{
    for (const MenuItem* menuItem : m_menuItems) {
        if (menuItem->id() == menuId) {
            dispatcher()->dispatch(menuItem->action().code, menuItem->args());
            return;
        }
    }
}

QString ToolBarItem::id() const
{
    return m_id;
}

QString ToolBarItem::translatedTitle() const
{
    return m_action.title.qTranslatedWithoutMnemonic();
}

UiActionState ToolBarItem::state() const
{
    return m_state;
}

bool ToolBarItem::selected() const
{
    return m_selected;
}

ToolBarItemType::Type ToolBarItem::type() const
{
    return m_type;
}

QList<MenuItem*> ToolBarItem::menuItems() const
{
    return m_menuItems;
}

UiAction ToolBarItem::action() const
{
    return m_action;
}

muse::actions::ActionData ToolBarItem::args() const
{
    return m_args;
}

bool ToolBarItem::isValid() const
{
    return !m_id.isEmpty();
}

QString ToolBarItem::shortcutsTitle() const
{
    return shortcuts::sequencesToNativeText(m_action.shortcuts);
}

void ToolBarItem::setId(const QString& id)
{
    if (m_id == id) {
        return;
    }

    m_id = id;
    emit idChanged(m_id);
}

void ToolBarItem::setTitle(const TranslatableString& title)
{
    if (m_action.title == title) {
        return;
    }

    m_action.title = title;
    emit actionChanged();
}

void ToolBarItem::setState(const UiActionState& state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    emit stateChanged();
}

void ToolBarItem::setSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }

    m_selected = selected;
    emit selectedChanged(m_selected);
}

void ToolBarItem::setType(ToolBarItemType::Type type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    emit typeChanged(type_property());
}

void ToolBarItem::setMenuItems(const QList<MenuItem*>& menuItems)
{
    if (m_menuItems == menuItems) {
        return;
    }

    m_menuItems = menuItems;
    emit menuItemsChanged(m_menuItems, m_id);
}

void ToolBarItem::setAction(const UiAction& action)
{
    if (m_action == action) {
        return;
    }

    m_action = action;
    emit actionChanged();
}

void ToolBarItem::setArgs(const muse::actions::ActionData& args)
{
    m_args = args;
}

QString muse::uicomponents::ToolBarItem::code_property() const
{
    return QString::fromStdString(m_action.code);
}

QString ToolBarItem::description_property() const
{
    return m_action.description.qTranslated();
}

int ToolBarItem::icon_property() const
{
    return static_cast<int>(m_action.iconCode);
}

bool ToolBarItem::enabled_property() const
{
    return m_state.enabled;
}

bool ToolBarItem::checkable_property() const
{
    return m_action.checkable == Checkable::Yes;
}

bool ToolBarItem::checked_property() const
{
    return m_state.checked;
}

bool ToolBarItem::selected_property() const
{
    return m_selected;
}

int ToolBarItem::type_property() const
{
    return static_cast<int>(m_type);
}

bool ToolBarItem::isMenuSecondary() const
{
    return m_isMenuSecondary;
}

void ToolBarItem::setIsMenuSecondary(bool secondary)
{
    if (m_isMenuSecondary == secondary) {
        return;
    }

    m_isMenuSecondary = secondary;
    emit isMenuSecondaryChanged(secondary);
}

bool ToolBarItem::showTitle() const
{
    return m_showTitle;
}

void ToolBarItem::setShowTitle(bool show)
{
    if (m_showTitle == show) {
        return;
    }

    m_showTitle = show;
    emit showTitleChanged();
}

bool ToolBarItem::isTitleBold() const
{
    return m_isTitleBold;
}

void ToolBarItem::setIsTitleBold(bool newIsTitleBold)
{
    if (m_isTitleBold == newIsTitleBold) {
        return;
    }

    m_isTitleBold = newIsTitleBold;
    emit isTitleBoldChanged();
}

bool ToolBarItem::isTransparent() const
{
    return m_isTransparent;
}

void ToolBarItem::setIsTransparent(bool isTransparent)
{
    if (m_isTransparent == isTransparent) {
        return;
    }

    m_isTransparent = isTransparent;
    emit isTransparentChanged();
}
