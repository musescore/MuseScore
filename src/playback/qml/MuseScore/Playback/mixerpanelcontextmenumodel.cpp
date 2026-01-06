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

#include "mixerpanelcontextmenumodel.h"

#include "types/translatablestring.h"

using namespace mu;
using namespace mu::playback;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::actions;
using namespace muse::audio;

static const ActionCode TOGGLE_MIXER_SECTION_ACTION("toggle-mixer-section");
static const ActionCode TOGGLE_AUX_SEND_ACTION("toggle-aux-send");
static const ActionCode TOGGLE_AUX_CHANNEL_ACTION("toggle-aux-channel");

static const QString VIEW_MENU_ID("view-menu");

static TranslatableString mixerSectionTitle(MixerSectionType type)
{
    switch (type) {
    case MixerSectionType::Labels: return TranslatableString("playback", "Labels");
    case MixerSectionType::Sound: return TranslatableString("playback", "Sound");
    case MixerSectionType::AudioFX: return TranslatableString("playback", "Audio FX");
    case MixerSectionType::Balance: return TranslatableString("playback", "Pan");
    case MixerSectionType::Volume: return TranslatableString("playback", "Volume");
    case MixerSectionType::Fader: return TranslatableString("playback", "Fader");
    case MixerSectionType::MuteAndSolo: return TranslatableString("playback", "Mute and solo");
    case MixerSectionType::Title: return TranslatableString("playback", "Name");
    case MixerSectionType::Unknown: break;
    }

    return {};
}

static QString auxSendVisibleMenuItemId(aux_channel_idx_t index)
{
    return QString("aux-send-%1-visible").arg(index);
}

static QString auxChannelVisibleMenuItemId(aux_channel_idx_t index)
{
    return QString("aux-channel-%1-visible").arg(index);
}

MixerPanelContextMenuModel::MixerPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

bool MixerPanelContextMenuModel::labelsSectionVisible() const
{
    return isSectionVisible(MixerSectionType::Labels);
}

bool MixerPanelContextMenuModel::soundSectionVisible() const
{
    return isSectionVisible(MixerSectionType::Sound);
}

bool MixerPanelContextMenuModel::audioFxSectionVisible() const
{
    return isSectionVisible(MixerSectionType::AudioFX);
}

bool MixerPanelContextMenuModel::auxSendsSectionVisible() const
{
    for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
        if (configuration()->isAuxSendVisible(idx)) {
            return true;
        }
    }

    return false;
}

bool MixerPanelContextMenuModel::balanceSectionVisible() const
{
    return isSectionVisible(MixerSectionType::Balance);
}

bool MixerPanelContextMenuModel::volumeSectionVisible() const
{
    return isSectionVisible(MixerSectionType::Volume);
}

bool MixerPanelContextMenuModel::faderSectionVisible() const
{
    return isSectionVisible(MixerSectionType::Fader);
}

bool MixerPanelContextMenuModel::muteAndSoloSectionVisible() const
{
    return isSectionVisible(MixerSectionType::MuteAndSolo);
}

bool MixerPanelContextMenuModel::titleSectionVisible() const
{
    return isSectionVisible(MixerSectionType::Title);
}

void MixerPanelContextMenuModel::load()
{
    AbstractMenuModel::load();

    dispatcher()->reg(this, TOGGLE_MIXER_SECTION_ACTION, this, &MixerPanelContextMenuModel::toggleMixerSection);
    dispatcher()->reg(this, TOGGLE_AUX_SEND_ACTION, this, &MixerPanelContextMenuModel::toggleAuxSend);
    dispatcher()->reg(this, TOGGLE_AUX_CHANNEL_ACTION, this, &MixerPanelContextMenuModel::toggleAuxChannel);

    configuration()->isAuxSendVisibleChanged().onReceive(this, [this](aux_channel_idx_t auxSendIndex, bool newVisibilityValue) {
        setViewMenuItemChecked(auxSendVisibleMenuItemId(auxSendIndex), newVisibilityValue);

        emit auxSendsSectionVisibleChanged();
    });

    configuration()->isAuxChannelVisibleChanged().onReceive(this, [this](aux_channel_idx_t auxChannelIndex, bool newVisibilityValue) {
        setViewMenuItemChecked(auxChannelVisibleMenuItemId(auxChannelIndex), newVisibilityValue);
    });

    configuration()->isMixerSectionVisibleChanged().onReceive(this, [this](MixerSectionType sectionType, bool newVisibilityValue) {
        setViewMenuItemChecked(QString::number(static_cast<int>(sectionType)), newVisibilityValue);

        emitMixerSectionVisibilityChanged(sectionType);
    });

    MenuItemList viewMenuItems {
        buildSectionVisibleItem(MixerSectionType::Labels),
        buildSectionVisibleItem(MixerSectionType::Sound),
        buildSectionVisibleItem(MixerSectionType::AudioFX),
    };

    for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
        viewMenuItems.push_back(buildAuxSendVisibleItem(idx));
    }

    for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
        viewMenuItems.push_back(buildAuxChannelVisibleItem(idx));
    }

    viewMenuItems.push_back(buildSectionVisibleItem(MixerSectionType::Balance));
    viewMenuItems.push_back(buildSectionVisibleItem(MixerSectionType::Volume));
    viewMenuItems.push_back(buildSectionVisibleItem(MixerSectionType::Fader));
    viewMenuItems.push_back(buildSectionVisibleItem(MixerSectionType::MuteAndSolo));
    viewMenuItems.push_back(buildSectionVisibleItem(MixerSectionType::Title));

    MenuItemList items {
        makeMenuItem("playback-setup"),
        makeMenu(TranslatableString("playback", "View"), viewMenuItems, VIEW_MENU_ID)
    };

    setItems(items);
}

bool MixerPanelContextMenuModel::isSectionVisible(MixerSectionType sectionType) const
{
    return configuration()->isMixerSectionVisible(sectionType);
}

MenuItem* MixerPanelContextMenuModel::buildSectionVisibleItem(MixerSectionType sectionType)
{
    int sectionTypeInt = static_cast<int>(sectionType);

    MenuItem* item = new MenuItem(this);
    item->setId(QString::number(sectionTypeInt));
    item->setArgs(ActionData::make_arg1<int>(sectionTypeInt));

    UiAction action;
    action.title = mixerSectionTitle(sectionType);
    action.code = TOGGLE_MIXER_SECTION_ACTION;
    action.checkable = Checkable::Yes;
    item->setAction(action);

    UiActionState state;
    state.enabled = true;
    state.checked = isSectionVisible(sectionType);
    item->setState(state);

    return item;
}

MenuItem* MixerPanelContextMenuModel::buildAuxSendVisibleItem(aux_channel_idx_t index)
{
    MenuItem* item = new MenuItem(this);
    item->setId(auxSendVisibleMenuItemId(index));
    item->setArgs(ActionData::make_arg1<int>(index));

    UiAction action;
    action.title = TranslatableString("playback", String("Aux send %1").arg(index + 1));
    action.code = TOGGLE_AUX_SEND_ACTION;
    action.checkable = Checkable::Yes;
    item->setAction(action);

    UiActionState state;
    state.enabled = true;
    state.checked = configuration()->isAuxSendVisible(index);
    item->setState(state);

    return item;
}

MenuItem* MixerPanelContextMenuModel::buildAuxChannelVisibleItem(aux_channel_idx_t index)
{
    MenuItem* item = new MenuItem(this);
    item->setId(auxChannelVisibleMenuItemId(index));
    item->setArgs(ActionData::make_arg1<int>(index));

    UiAction action;
    action.title = TranslatableString("playback", String("Aux channel %1").arg(index + 1));
    action.code = TOGGLE_AUX_CHANNEL_ACTION;
    action.checkable = Checkable::Yes;
    item->setAction(action);

    UiActionState state;
    state.enabled = true;
    state.checked = configuration()->isAuxChannelVisible(index);
    item->setState(state);

    return item;
}

void MixerPanelContextMenuModel::toggleMixerSection(const ActionData& args)
{
    if (args.empty()) {
        return;
    }

    int sectionTypeInt = args.arg<int>(0);
    MixerSectionType sectionType = static_cast<MixerSectionType>(sectionTypeInt);

    bool newVisibilityValue = !isSectionVisible(sectionType);
    configuration()->setMixerSectionVisible(sectionType, newVisibilityValue);
}

void MixerPanelContextMenuModel::toggleAuxSend(const ActionData& args)
{
    if (args.empty()) {
        return;
    }

    aux_channel_idx_t auxSendIndex = static_cast<aux_channel_idx_t>(args.arg<int>(0));
    bool newVisibilityValue = !configuration()->isAuxSendVisible(auxSendIndex);

    configuration()->setAuxSendVisible(auxSendIndex, newVisibilityValue);
}

void MixerPanelContextMenuModel::toggleAuxChannel(const ActionData& args)
{
    if (args.empty()) {
        return;
    }

    aux_channel_idx_t auxChannelIndex = static_cast<aux_channel_idx_t>(args.arg<int>(0));
    bool newVisibilityValue = !configuration()->isAuxChannelVisible(auxChannelIndex);

    configuration()->setAuxChannelVisible(auxChannelIndex, newVisibilityValue);
}

void MixerPanelContextMenuModel::setViewMenuItemChecked(const QString& itemId, bool checked)
{
    MenuItem& viewMenu = findMenu(VIEW_MENU_ID);

    for (MenuItem* item : viewMenu.subitems()) {
        if (item->id() == itemId) {
            UiActionState state = item->state();
            state.checked = checked;
            item->setState(state);
            return;
        }
    }
}

void MixerPanelContextMenuModel::emitMixerSectionVisibilityChanged(MixerSectionType sectionType)
{
    switch (sectionType) {
    case MixerSectionType::Labels:
        emit labelsSectionVisibleChanged();
        break;
    case MixerSectionType::Sound:
        emit soundSectionVisibleChanged();
        break;
    case MixerSectionType::AudioFX:
        emit audioFxSectionVisibleChanged();
        break;
    case MixerSectionType::Balance:
        emit balanceSectionVisibleChanged();
        break;
    case MixerSectionType::Volume:
        emit volumeSectionVisibleChanged();
        break;
    case MixerSectionType::Fader:
        emit faderSectionVisibleChanged();
        break;
    case MixerSectionType::MuteAndSolo:
        emit muteAndSoloSectionVisibleChanged();
        break;
    case MixerSectionType::Title:
        emit titleSectionVisibleChanged();
        break;
    case MixerSectionType::Unknown:
        break;
    }
}
