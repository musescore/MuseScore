/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#include "pianokeyboardpanelcontextmenumodel.h"

#include "log.h"

#include "actions/actiontypes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::ui;
using namespace mu::uicomponents;

static const ActionCode SET_NUMBER_OF_KEYS_CODE("piano-keyboard-set-number-of-keys");

PianoKeyboardPanelContextMenuModel::PianoKeyboardPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void PianoKeyboardPanelContextMenuModel::load()
{
    MenuItemList items {
        makeViewMenu()
    };

    setItems(items);
}

int PianoKeyboardPanelContextMenuModel::numberOfKeys() const
{
    return configuration()->pianoKeyboardNumberOfKeys().val;
}

MenuItem* PianoKeyboardPanelContextMenuModel::makeViewMenu()
{
    MenuItemList items;

    configuration()->pianoKeyboardNumberOfKeys().ch.onReceive(this, [this](int) {
        emit numberOfKeysChanged();
    });

    std::vector<std::pair<QString, int> > possibleNumbersOfKeys {
        { qtrc("notation", "128 notes (full)"), 128 },
        { qtrc("notation", "88 notes (piano)"), 88 },
        { qtrc("notation", "61 notes"), 61 },
        { qtrc("notation", "49 notes"), 49 },
        { qtrc("notation", "25 notes"), 25 },
    };

    for (auto [title, numberOfKeys] : possibleNumbersOfKeys) {
        items << makeNumberOfKeysItem(title, numberOfKeys);
    }

    dispatcher()->reg(this, SET_NUMBER_OF_KEYS_CODE, [this](const ActionData& args) {
        IF_ASSERT_FAILED(args.count() > 0) {
            return;
        }

        configuration()->setPianoKeyboardNumberOfKeys(args.arg<int>(0));
    });

    return makeMenu(qtrc("notation", "View"), items);
}

MenuItem* PianoKeyboardPanelContextMenuModel::makeNumberOfKeysItem(const QString& title, int numberOfKeys)
{
    UiAction action;
    action.title = title;
    action.code = SET_NUMBER_OF_KEYS_CODE;

    MenuItem* item = new MenuItem(action, this);
    item->setId(QString::number(numberOfKeys));

    ValCh<int> currentNumberOfKeys = configuration()->pianoKeyboardNumberOfKeys();

    item->setState(UiActionState::make_enabled());
    item->setSelectable(true);
    item->setSelected(numberOfKeys == currentNumberOfKeys.val);

    currentNumberOfKeys.ch.onReceive(item, [item, numberOfKeys](int num) {
        item->setSelected(numberOfKeys == num);
    });

    item->setArgs(ActionData::make_arg1<int>(numberOfKeys));

    m_numberOfKeysItems << item;

    return item;
}
