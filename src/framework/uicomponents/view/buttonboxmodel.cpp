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
    : QAbstractListModel(parent)
{
}

void ButtonBoxModel::load()
{
    beginResetModel();

    std::unordered_map <int, std::vector <LayoutButton*> > sortedButtons;

    for (int i : m_buttonsEnum) {
        LayoutButton* button = m_layoutButtons[ButtonType(i)];

        sortedButtons[button->buttonRole].push_back(button);
    }

    for (LayoutButton* button : m_customButtons) {
        sortedButtons[button->buttonRole].push_back(button);
    }

    const std::vector <ButtonRole> currentLayout = chooseButtonLayoutType();

    for (ButtonRole buttonRole : currentLayout) {
        std::vector <LayoutButton*> buttons = sortedButtons[buttonRole];

        for (LayoutButton* button : buttons) {
            m_buttons << button;
        }
    }

    std::vector <LayoutButton*> acceptApplyButtons(sortedButtons[ButtonRole::AcceptRole]);
    acceptApplyButtons.insert(acceptApplyButtons.end(),
                              sortedButtons[ButtonRole::ApplyRole].begin(), sortedButtons[ButtonRole::ApplyRole].end());

    LayoutButton* targetButton = acceptApplyButtons.size() > 0 ? acceptApplyButtons[0] : nullptr;
    for (LayoutButton* button : acceptApplyButtons) { // Searching for accent button, accent button be navigation column 0
        if (button->isAccent) {
            targetButton = button;
            break;
        }
    }

    if (!targetButton) {
        endResetModel();
        return;
    }

    auto it = std::find(m_buttons.begin(), m_buttons.end(), targetButton);

    int navigationColumnStart = 0;
    if (it != m_buttons.end()) {
        navigationColumnStart = it - m_buttons.begin();
    }

    for (int i = 0; i < m_buttons.size(); i++) {
        LayoutButton* button = m_buttons[i];

        button->navigationColumn = (i + m_buttons.size() - navigationColumnStart) % m_buttons.size(); // Algorithm for "shifting"
                                                                                                      // indexes
    }

    endResetModel();
}

QVariant ButtonBoxModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    LayoutButton* button = m_buttons[index.row()];

    switch (role) {
    case ButtonText:
        return button->text;
    case Type:
        return button->buttonType;
    case Accent:
        return button->isAccent;
    case IsLeftSide:
        return button->isLeftSide;
    case CustomButtonIndex:
        return button->customButtonIndex;
    case NavigationName:
        return button->navigationName;
    case NavigationColumn:
        return button->navigationColumn;
    }

    return QVariant();
}

int ButtonBoxModel::rowCount(const QModelIndex&) const
{
    return m_buttons.size();
}

QHash<int, QByteArray> ButtonBoxModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { ButtonText, "text" },
        { Type, "type" },
        { Accent, "isAccent" },
        { IsLeftSide, "isLeftSide" },
        { CustomButtonIndex, "customButtonIndex" },
        { NavigationName, "navigationName" },
        { NavigationColumn, "navigationColumn" }
    };

    return roles;
}

int ButtonBoxModel::buttons() const
{
    return 0; // Not used
}

void ButtonBoxModel::setButtons(const int& buttons)
{
    int i = ButtonType::FirstButton;
    while (i <= ButtonType::LastButton) {
        if (i & buttons) {
            m_buttonsEnum << ButtonType(i);
        }
        i = i << 1;
    }
}

const std::vector <ButtonBoxModel::ButtonRole> ButtonBoxModel::chooseButtonLayoutType()
{
    int index = 0;
#if defined (Q_OS_OSX)
    return buttonRoleLayouts[1];
    index = 1;
#elif defined (Q_OS_LINUX) || defined (Q_OS_UNIX)
    return buttonRoleLayouts[2];
    index = 2;
#endif
    return buttonRoleLayouts[index];
}

void ButtonBoxModel::addCustomButton(int index, QString text, int role, bool isAccent, bool isLeftSide, QString navigationName)
{
    LayoutButton* button = new LayoutButton(text, ButtonType::CustomButton,  ButtonRole(role), isAccent, isLeftSide, navigationName);
    button->customButtonIndex = index;
    m_customButtons << button;
}
