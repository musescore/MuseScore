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

static const ActionCode SET_KEY_WIDTH_SCALING_CODE("piano-keyboard-set-key-width-scaling");
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

qreal PianoKeyboardPanelContextMenuModel::keyWidthScaling() const
{
    return m_currentKeyWidthScaling;
}

void PianoKeyboardPanelContextMenuModel::setKeyWidthScaling(qreal scaling)
{
    if (qFuzzyCompare(m_currentKeyWidthScaling, scaling)) {
        return;
    }

    m_currentKeyWidthScaling = scaling;
    updateKeyWidthScalingItems();
    emit keyWidthScalingChanged();
}

int PianoKeyboardPanelContextMenuModel::numberOfKeys() const
{
    return configuration()->pianoKeyboardNumberOfKeys().val;
}

MenuItem* PianoKeyboardPanelContextMenuModel::makeViewMenu()
{
    MenuItemList items;

    std::vector<std::pair<QString, qreal> > possibleKeyWidthScalings {
        { qtrc("notation", "Large"), 2.0 },
        { qtrc("notation", "Normal"), 1.0 },
        { qtrc("notation", "Small"), 0.5 },
    };

    for (auto [title, scaling] : possibleKeyWidthScalings) {
        items << makeKeyWidthScalingItem(title, scaling);
    }

    updateKeyWidthScalingItems();

    dispatcher()->reg(this, SET_KEY_WIDTH_SCALING_CODE, [this](const ActionData& args) {
        IF_ASSERT_FAILED(args.count() > 0) {
            return;
        }

        emit setKeyWidthScalingRequested(args.arg<qreal>(0));
    });

    items << makeSeparator();

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

    configuration()->pianoKeyboardNumberOfKeys().ch.onReceive(this, [this](int) {
        emit numberOfKeysChanged();
    });

    dispatcher()->reg(this, SET_NUMBER_OF_KEYS_CODE, [this](const ActionData& args) {
        IF_ASSERT_FAILED(args.count() > 0) {
            return;
        }

        configuration()->setPianoKeyboardNumberOfKeys(args.arg<int>(0));
    });

    return makeMenu(qtrc("notation", "View"), items);
}

MenuItem* PianoKeyboardPanelContextMenuModel::makeKeyWidthScalingItem(const QString& title, qreal scaling)
{
    UiAction action;
    action.title = title;
    action.code = SET_KEY_WIDTH_SCALING_CODE;
    action.checkable = Checkable::Yes;

    MenuItem* item = new MenuItem(action, this);
    item->setId(QString::number(scaling));
    item->setState(UiActionState::make_enabled());
    item->setArgs(ActionData::make_arg1<qreal>(scaling));

    m_keyWidthScalingItems << item;

    return item;
}

MenuItem* PianoKeyboardPanelContextMenuModel::makeNumberOfKeysItem(const QString& title, int numberOfKeys)
{
    UiAction action;
    action.title = title;
    action.code = SET_NUMBER_OF_KEYS_CODE;
    action.checkable = Checkable::Yes;

    MenuItem* item = new MenuItem(action, this);
    item->setId(QString::number(numberOfKeys));

    ValCh<int> currentNumberOfKeys = configuration()->pianoKeyboardNumberOfKeys();

    bool checked = numberOfKeys == currentNumberOfKeys.val;
    item->setState(UiActionState::make_enabled(checked));

    currentNumberOfKeys.ch.onReceive(item, [item, numberOfKeys](int num) {
        bool checked = numberOfKeys == num;
        item->setState(UiActionState::make_enabled(checked));
    });

    item->setArgs(ActionData::make_arg1<int>(numberOfKeys));

    return item;
}

void PianoKeyboardPanelContextMenuModel::updateKeyWidthScalingItems()
{
    qreal roundedCurrentScaling = 1.0;
    if (m_currentKeyWidthScaling < 0.75) {
        roundedCurrentScaling = 0.5;
    } else if (m_currentKeyWidthScaling > 1.50) {
        roundedCurrentScaling = 2.0;
    }

    for (MenuItem* item : m_keyWidthScalingItems) {
        IF_ASSERT_FAILED(item->args().count() > 0) {
            continue;
        }

        bool checked = qFuzzyCompare(item->args().arg<qreal>(0), roundedCurrentScaling);
        item->setState(UiActionState::make_enabled(checked));
    }
}
