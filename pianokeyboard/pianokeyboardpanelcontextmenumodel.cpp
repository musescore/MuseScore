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

#include "actions/actiontypes.h"
#include "pianokeyboardtypes.h"
#include "types/translatablestring.h"

#include "log.h"

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

    std::vector<std::pair<TranslatableString, qreal> > possibleKeyWidthScalings {
        { TranslatableString("notation", "Large"), LARGE_KEY_WIDTH_SCALING },
        { TranslatableString("notation", "Normal"), NORMAL_KEY_WIDTH_SCALING },
        { TranslatableString("notation", "Small"), SMALL_KEY_WIDTH_SCALING },
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

    std::vector<std::pair<TranslatableString, int> > possibleNumbersOfKeys {
        { TranslatableString("notation", "128 notes (full)"), 128 },
        { TranslatableString("notation", "88 notes (piano)"), 88 },
        { TranslatableString("notation", "61 notes"), 61 },
        { TranslatableString("notation", "49 notes"), 49 },
        { TranslatableString("notation", "25 notes"), 25 },
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

    return makeMenu(TranslatableString("notation", "View"), items);
}

MenuItem* PianoKeyboardPanelContextMenuModel::makeKeyWidthScalingItem(const TranslatableString& title, qreal scaling)
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

MenuItem* PianoKeyboardPanelContextMenuModel::makeNumberOfKeysItem(const TranslatableString& title, int numberOfKeys)
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
    qreal roundedCurrentScaling = NORMAL_KEY_WIDTH_SCALING;

    constexpr qreal betweenNormalAndSmall = (NORMAL_KEY_WIDTH_SCALING + SMALL_KEY_WIDTH_SCALING) / 2;
    constexpr qreal betweenNormalAndLarge = (NORMAL_KEY_WIDTH_SCALING + LARGE_KEY_WIDTH_SCALING) / 2;

    if (m_currentKeyWidthScaling < betweenNormalAndSmall) {
        roundedCurrentScaling = SMALL_KEY_WIDTH_SCALING;
    } else if (m_currentKeyWidthScaling > betweenNormalAndLarge) {
        roundedCurrentScaling = LARGE_KEY_WIDTH_SCALING;
    }

    for (MenuItem* item : m_keyWidthScalingItems) {
        IF_ASSERT_FAILED(item->args().count() > 0) {
            continue;
        }

        bool checked = qFuzzyCompare(item->args().arg<qreal>(0), roundedCurrentScaling);
        item->setState(UiActionState::make_enabled(checked));
    }
}
