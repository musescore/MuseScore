/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "mixerchannelcontextmenumodel.h"

#include "mixerpanelmodel.h"
#include "types/translatablestring.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::uicomponents;

MixerChannelContextMenuModel::MixerChannelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void MixerChannelContextMenuModel::loadForChannel(int channelIndex, QObject* mixerModelObj)
{
    m_channelIndex = channelIndex;
    m_mixerModel = qobject_cast<MixerPanelModel*>(mixerModelObj);

    buildMenu();

    emit canPasteChanged();
}

bool MixerChannelContextMenuModel::canPaste() const
{
    return m_mixerModel && m_mixerModel->hasClipboardData();
}

bool MixerChannelContextMenuModel::canUndo() const
{
    return m_mixerModel && m_mixerModel->canUndo();
}

bool MixerChannelContextMenuModel::canRedo() const
{
    return m_mixerModel && m_mixerModel->canRedo();
}

void MixerChannelContextMenuModel::buildMenu()
{
    MenuItemList items;

    // Copy item
    MenuItem* copyItem = new MenuItem(this);
    copyItem->setId("copy");

    ui::UiAction copyAction;
    copyAction.title = TranslatableString("playback", "Copy channel settings");
    copyAction.code = "copy";
    copyItem->setAction(copyAction);

    ui::UiActionState copyState;
    copyState.enabled = true;
    copyItem->setState(copyState);

    items << copyItem;

    // Paste item
    MenuItem* pasteItem = new MenuItem(this);
    pasteItem->setId("paste");

    ui::UiAction pasteAction;
    pasteAction.title = TranslatableString("playback", "Paste channel settings");
    pasteAction.code = "paste";
    pasteItem->setAction(pasteAction);

    ui::UiActionState pasteState;
    pasteState.enabled = canPaste();
    pasteItem->setState(pasteState);

    items << pasteItem;

    // Separator
    items << makeSeparator();

    // Apply to all item
    MenuItem* applyToAllItem = new MenuItem(this);
    applyToAllItem->setId("apply-to-all");

    ui::UiAction applyAction;
    applyAction.title = TranslatableString("playback", "Apply to all channels");
    applyAction.code = "apply-to-all";
    applyToAllItem->setAction(applyAction);

    ui::UiActionState applyState;
    applyState.enabled = true;
    applyToAllItem->setState(applyState);

    items << applyToAllItem;

    // Separator before undo/redo
    items << makeSeparator();

    // Undo item
    MenuItem* undoItem = new MenuItem(this);
    undoItem->setId("undo");

    ui::UiAction undoAction;
    undoAction.title = TranslatableString("playback", "Undo");
    undoAction.code = "undo";
    undoItem->setAction(undoAction);

    ui::UiActionState undoState;
    undoState.enabled = canUndo();
    undoItem->setState(undoState);

    items << undoItem;

    // Redo item
    MenuItem* redoItem = new MenuItem(this);
    redoItem->setId("redo");

    ui::UiAction redoAction;
    redoAction.title = TranslatableString("playback", "Redo");
    redoAction.code = "redo";
    redoItem->setAction(redoAction);

    ui::UiActionState redoState;
    redoState.enabled = canRedo();
    redoItem->setState(redoState);

    items << redoItem;

    setItems(items);
}

