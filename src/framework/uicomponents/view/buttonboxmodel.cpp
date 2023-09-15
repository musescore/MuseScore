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

#include "buttonboxmodel.h"

#include "log.h"

using namespace mu::uicomponents;

ButtonBoxModel::ButtonBoxModel(QObject* parent)
    : QObject(parent), m_buttonsItems(this)
{
}

QList<int> ButtonBoxModel::load()
{
    QList<int> result;
    std::unordered_map <int, std::vector <LayoutButton*> > sortedButtons;

    QList<QQuickItem*> buttonsItems = m_buttonsItems.list();
    for (const QQuickItem* item : buttonsItems) {
        QVariant buttonRoleVar = item->property("buttonRole");
        if (!buttonRoleVar.isValid()) {
            continue;
        }

        ButtonRole role = static_cast<ButtonRole>(buttonRoleVar.toInt());

        QVariant buttonTypeVar = item->property("buttonId");
        int type = static_cast<int>(ButtonType::CustomButton);
        LayoutButton* button = nullptr;
        if (buttonTypeVar.isValid()) {
            type = buttonTypeVar.toInt();
            if (type >= static_cast<int>(ButtonType::CustomButton)) {
                button = layoutButton(item);
            } else {
                button = m_layoutButtons[static_cast<ButtonType>(type)];
            }
        } else {
            button = m_layoutButtons[static_cast<ButtonType>(type)];
        }

        sortedButtons[role].push_back(button);
    }

    const std::vector<ButtonRole> currentLayout = chooseButtonLayoutType();

    for (ButtonRole buttonRole : currentLayout) {
        if (!contains(sortedButtons, static_cast<int>(buttonRole))) {
            continue;
        }

        std::vector<LayoutButton*> buttons = sortedButtons[buttonRole];

        for (LayoutButton* button : buttons) {
            result << button->buttonType;
        }
    }

    return result;
}

void ButtonBoxModel::setButtons(const QVariantList& buttons)
{
    for (const QVariant& buttonTypeVar : buttons) {
        LayoutButton* button = m_layoutButtons[ButtonType(buttonTypeVar.toInt())];
        emit addButton(button->text, button->buttonType, button->buttonRole, button->isAccent, button->isLeftSide);
    }
}

const std::vector <ButtonBoxModel::ButtonRole> ButtonBoxModel::chooseButtonLayoutType()
{
    size_t index = 0;
    if (m_buttonLayout != ButtonLayout::UnknownLayout) {
        index = static_cast<size_t>(m_buttonLayout);
    } else {
#if defined (Q_OS_OSX)
        index = 1;
#elif defined (Q_OS_LINUX) || defined (Q_OS_UNIX)
        index = 2;
#endif
    }

    IF_ASSERT_FAILED(index >= 0 || index <= buttonRoleLayouts.size()) {
        index = 0;
    }

    return buttonRoleLayouts[index];
}

ButtonBoxModel::LayoutButton* ButtonBoxModel::layoutButton(const QQuickItem* item) const
{
    QString text = item->property("text").toString();
    ButtonRole role = static_cast<ButtonRole>(item->property("buttonRole").toInt());
    int type = item->property("buttonId").toInt();
    bool isAccent = item->property("accentButton").toBool();
    bool isLeftSide = item->property("isLeftSide").toBool();

    return new LayoutButton(text, type, role, isAccent, isLeftSide);
}

ButtonBoxModel::ButtonLayout ButtonBoxModel::buttonLayout() const
{
    return m_buttonLayout;
}

void ButtonBoxModel::setButtonLayout(ButtonLayout newButtonLayout)
{
    if (m_buttonLayout == newButtonLayout) {
        return;
    }
    m_buttonLayout = newButtonLayout;
    emit buttonLayoutChanged();

    emit reloadRequested();
}

QQmlListProperty<QQuickItem> ButtonBoxModel::buttonsItems()
{
    return m_buttonsItems.property();
}
