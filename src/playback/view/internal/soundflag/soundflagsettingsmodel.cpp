/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include "soundflagsettingsmodel.h"

#include "engraving/types/types.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/soundflag.h"

#include "audio/audioutils.h"
#include "actions/actiontypes.h"

#include "translation.h"
#include "log.h"

using namespace mu;
using namespace mu::playback;

static const QString RESET_MENU_ID = "reset";
static const QString MULTI_SELECTION_MENU_ID = "multi-selection";

SoundFlagSettingsModel::SoundFlagSettingsModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_SOUND_FLAG, parent)
{
}

void SoundFlagSettingsModel::init()
{
    TRACEFUNC;

    connect(this, &SoundFlagSettingsModel::itemRectChanged, this, [this](const QRect&) {
        emit iconRectChanged();
    });

    AbstractElementPopupModel::init();

    IF_ASSERT_FAILED(m_item && m_item->isSoundFlag()) {
        return;
    }

    initTitle();
    initAvailablePresets();
    initAvailablePlayingTechniques();

    emit showTextChanged();
    emit textChanged();
    emit contextMenuModelChanged();
}

void SoundFlagSettingsModel::initTitle()
{
    const audio::AudioInputParams& params = currentAudioInputParams();
    audio::AudioSourceType type = params.type();

    QString title;
    QString name = audio::audioSourceName(params).toQString();
    QString category = audio::audioSourceCategoryName(params).toQString();

    if (audio::AudioSourceType::MuseSampler == type) {
        title = category + ": " + name;
    }

    setTitle(title);
}

void SoundFlagSettingsModel::initAvailablePresets()
{
    playbackController()->availableSoundPresets(makeInstrumentTrackId(m_item))
    .onResolve(this, [this](const audio::SoundPresetList& presets) {
        setAvailablePresets(presets);
    });

    emit presetCodesChanged();
}

void SoundFlagSettingsModel::initAvailablePlayingTechniques()
{
    NOT_IMPLEMENTED;
}

void SoundFlagSettingsModel::togglePreset(const QString& presetCode)
{
    if (!m_item) {
        return;
    }

    QStringList presetCodes = this->presetCodes();

    if (playbackConfiguration()->isSoundFlagsMultiSelectionEnabled()) {
        if (presetCodes.contains(presetCode)) {
            if (presetCodes.size() == 1) {
                return;
            }

            presetCodes.removeAll(presetCode);
        } else {
            presetCodes.push_back(presetCode);
        }
    } else {
        presetCodes = QStringList{ presetCode };
    }

    engraving::SoundFlag* soundFlag = engraving::toSoundFlag(m_item);

    beginCommand();
    soundFlag->undoChangeSoundFlag(StringList(presetCodes), soundFlag->playingTechniques());
    updateStaffText();
    endCommand();

    updateNotation();

    emit presetCodesChanged();
}

void SoundFlagSettingsModel::togglePlayingTechnique(const QString& playingTechniqueCode)
{
    if (!m_item) {
        return;
    }

    QStringList playingTechniquesCodes = this->playingTechniquesCodes();

    if (playbackConfiguration()->isSoundFlagsMultiSelectionEnabled()) {
        if (playingTechniquesCodes.contains(playingTechniqueCode)) {
            if (playingTechniquesCodes.size() == 1) {
                return;
            }

            playingTechniquesCodes.removeAll(playingTechniqueCode);
        } else {
            playingTechniquesCodes.push_back(playingTechniqueCode);
        }
    } else {
        playingTechniquesCodes = QStringList{ playingTechniqueCode };
    }

    engraving::SoundFlag* soundFlag = engraving::toSoundFlag(m_item);

    beginCommand();

    soundFlag->undoChangeSoundFlag(soundFlag->soundPresets(), StringList(playingTechniquesCodes));
    bool needUpdateNotation = updateStaffText();

    endCommand();

    if (needUpdateNotation) {
        updateNotation();
    }

    emit playingTechniquesCodesChanged();
}

uicomponents::MenuItem* SoundFlagSettingsModel::buildMenuItem(const QString& actionCode, const TranslatableString& title)
{
    uicomponents::MenuItem* item = new uicomponents::MenuItem(this);
    item->setId(actionCode);

    ui::UiAction action;
    action.code = actions::codeFromQString(actionCode);
    action.title = title;
    item->setAction(action);

    ui::UiActionState state;
    state.enabled = true;
    item->setState(state);

    return item;
}

QVariantList SoundFlagSettingsModel::contextMenuModel()
{
    uicomponents::MenuItemList items;

    uicomponents::MenuItem* resetItem = buildMenuItem(RESET_MENU_ID, TranslatableString("playback", "Reset"));

    ui::UiAction resetAction = resetItem->action();
    resetAction.iconCode = ui::IconCode::Code::UNDO;
    resetItem->setAction(resetAction);

    items << resetItem;

    uicomponents::MenuItem* multiSelectionItem
        = buildMenuItem(MULTI_SELECTION_MENU_ID, TranslatableString("playback", "Allow multiple selection"));

    ui::UiAction multiSelectionAction = multiSelectionItem->action();
    multiSelectionAction.checkable = ui::Checkable::Yes;
    multiSelectionItem->setAction(multiSelectionAction);

    ui::UiActionState multiSelectionActionState = multiSelectionItem->state();
    multiSelectionActionState.checked = playbackConfiguration()->isSoundFlagsMultiSelectionEnabled();
    multiSelectionItem->setState(multiSelectionActionState);

    items << multiSelectionItem;

    return uicomponents::menuItemListToVariantList(items);
}

void SoundFlagSettingsModel::handleContextMenuItem(const QString& menuId)
{
    if (menuId == RESET_MENU_ID) {
        engraving::SoundFlag* soundFlag = engraving::toSoundFlag(m_item);

        beginCommand();

        soundFlag->undoChangeSoundFlag(StringList(), StringList());
        bool needUpdateNotation = updateStaffText();

        endCommand();

        if (needUpdateNotation) {
            updateNotation();
        }

        emit presetCodesChanged();
        emit playingTechniquesCodesChanged();
    } else if (menuId == MULTI_SELECTION_MENU_ID) {
        playbackConfiguration()->setIsSoundFlagsMultiSelectionEnabled(!playbackConfiguration()->isSoundFlagsMultiSelectionEnabled());
        emit contextMenuModelChanged();
    }
}

engraving::StaffText* SoundFlagSettingsModel::staffText() const
{
    engraving::EngravingItem* parent = m_item->parentItem();
    return parent && parent->isStaffText() ? engraving::toStaffText(parent) : nullptr;
}

bool SoundFlagSettingsModel::updateStaffText()
{
    String oldStaffText = this->text();

    String newText = mtrc("engraving", "Staff text");

    auto getParamsNames = [](const QVariantList& params, const StringList& selectedParams) -> String {
        if (selectedParams.empty()) {
            return {};
        }

        StringList paramsNames;
        for (const String& paramCode : selectedParams) {
            for (const QVariant& paramVar : params) {
                if (paramVar.toMap()["code"].toString() == paramCode) {
                    paramsNames.push_back(paramVar.toMap()["name"].toString().toLower());
                }
            }
        }

        return !paramsNames.empty() ? paramsNames.join(u", ") : u"";
    };

    String presetsNames = getParamsNames(m_availablePresets, engraving::toSoundFlag(m_item)->soundPresets());
    if (!presetsNames.empty()) {
        newText = presetsNames;
    }

    String playingTechniquesNames = getParamsNames(m_availablePlayingTechniques, engraving::toSoundFlag(m_item)->playingTechniques());
    if (!playingTechniquesNames.empty()) {
        newText = !presetsNames.empty() ? (presetsNames + u", " + playingTechniquesNames) : playingTechniquesNames;
    }

    bool isTextChanged = oldStaffText != newText;
    if (isTextChanged) {
        staffText()->undoChangeProperty(mu::engraving::Pid::TEXT, newText);
    }

    return isTextChanged;
}

project::IProjectAudioSettingsPtr SoundFlagSettingsModel::audioSettings() const
{
    IF_ASSERT_FAILED(globalContext()->currentProject()) {
        return nullptr;
    }

    return globalContext()->currentProject()->audioSettings();
}

const audio::AudioInputParams& SoundFlagSettingsModel::currentAudioInputParams() const
{
    return audioSettings()->trackInputParams(mu::engraving::makeInstrumentTrackId(m_item));
}

QString SoundFlagSettingsModel::title() const
{
    return m_title;
}

void SoundFlagSettingsModel::setTitle(const QString& title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

QString SoundFlagSettingsModel::text() const
{
    engraving::StaffText* staffText = this->staffText();
    return staffText ? staffText->xmlText().toQString() : QString();
}

void SoundFlagSettingsModel::setText(const QString& text)
{
    if (this->text() == text) {
        return;
    }

    changeItemProperty(mu::engraving::Pid::TEXT, text);
    emit textChanged();
}

QRect SoundFlagSettingsModel::iconRect() const
{
    return m_item ? fromLogical(m_item->canvasBoundingRect()).toQRect() : QRect();
}

QVariantList SoundFlagSettingsModel::availablePresets() const
{
    return m_availablePresets;
}

void SoundFlagSettingsModel::setAvailablePresets(const audio::SoundPresetList& presets)
{
    QVariantList presetsList;
    for (const audio::SoundPreset& preset : presets) {
        QVariantMap map;
        map["code"] = QString::fromStdString(preset.code);
        map["name"] = QString::fromStdString(preset.name);

        presetsList << map;
    }

    if (m_availablePresets == presetsList) {
        return;
    }

    m_availablePresets = presetsList;
    emit availablePresetsChanged();
}

QStringList SoundFlagSettingsModel::presetCodes() const
{
    if (!m_item) {
        return {};
    }

    QStringList availablePresetCodes;
    for (const QVariant& presetVar : m_availablePresets) {
        availablePresetCodes << presetVar.toMap()["code"].toString();
    }

    QStringList result;
    for (const String& presetCode : engraving::toSoundFlag(m_item)->soundPresets()) {
        QString code = presetCode.toQString();
        if (availablePresetCodes.contains(code)) {
            result.push_back(code);
        }
    }

    if (result.empty() && !availablePresetCodes.empty()) {
        result = QStringList{ availablePresetCodes.first() };
    }

    return result;
}

QVariantList SoundFlagSettingsModel::availablePlayingTechniques() const
{
    return m_availablePlayingTechniques;
}

void SoundFlagSettingsModel::setAvailablePlayingTechniques(const audio::SoundPreset::PlayingTechniqueList& playingTechniques)
{
    QVariantList playingTechniquesList;
    for (const audio::SoundPreset::PlayingTechnique& playingTechnique : playingTechniques) {
        QVariantMap map;
        map["code"] = QString::fromStdString(playingTechnique.code);
        map["name"] = QString::fromStdString(playingTechnique.name);

        playingTechniquesList << map;
    }

    if (m_availablePlayingTechniques == playingTechniquesList) {
        return;
    }

    m_availablePlayingTechniques = playingTechniquesList;
    emit availablePlayingTechniquesChanged();
}

QStringList SoundFlagSettingsModel::playingTechniquesCodes() const
{
    if (!m_item) {
        return {};
    }

    QStringList availablePlayingTechniquesCodes;
    for (const QVariant& playingTechniqueVar : m_availablePlayingTechniques) {
        availablePlayingTechniquesCodes << playingTechniqueVar.toMap()["code"].toString();
    }

    QStringList result;
    for (const String& playingTechniqueCode : engraving::toSoundFlag(m_item)->playingTechniques()) {
        QString code = playingTechniqueCode.toQString();
        if (availablePlayingTechniquesCodes.contains(code)) {
            result.push_back(code);
        }
    }

    if (result.empty() && !availablePlayingTechniquesCodes.empty()) {
        result = QStringList{ availablePlayingTechniquesCodes.first() };
    }

    return result;
}
