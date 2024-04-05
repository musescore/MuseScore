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
using namespace mu::engraving;
using namespace muse::audio;

static const QString RESET_MENU_ID = "reset";
static const QString MULTI_SELECTION_MENU_ID = "multi-selection";

static QVariantList buildAvailablePresetsModel(const SoundPresetList& availablePresets)
{
    QVariantList model;

    if (availablePresets.size() <= 1) {
        return model; // only default preset is available; don't show it
    }

    for (const SoundPreset& preset : availablePresets) {
        QVariantMap item;
        item["code"] = preset.code.toQString();
        item["name"] = preset.name.toQString();

        model << item;
    }

    return model;
}

static QVariantList buildAvailablePlayingTechniquesModel(const std::set<String>& availableTechniqueCodes)
{
    QVariantList model;

    if (availableTechniqueCodes.empty()) {
        return model;
    }

    QVariantMap ordinaryItem;
    ordinaryItem["code"] = QString::fromStdString(mu::mpe::ORDINARY_PLAYING_TECHNIQUE_CODE);
    ordinaryItem["name"] = mu::qtrc("playback", "Ord. (default)");
    model << ordinaryItem;

    for (const String& playingTechniqueCode : availableTechniqueCodes) {
        QVariantMap item;
        item["code"] = playingTechniqueCode.toQString();
        item["name"] = playingTechniqueCode.toQString();

        model << item;
    }

    return model;
}

SoundFlagSettingsModel::SoundFlagSettingsModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_SOUND_FLAG, parent)
{
}

bool SoundFlagSettingsModel::inited() const
{
    return m_availablePresetsInited;
}

void SoundFlagSettingsModel::init()
{
    TRACEFUNC;

    connect(this, &SoundFlagSettingsModel::itemRectChanged, this, [this](const QRect&) {
        QRect rect = iconRect();
        if (rect.isValid()) {
            emit iconRectChanged(rect);
        }
    });

    AbstractElementPopupModel::init();

    connect(this, &AbstractElementPopupModel::dataChanged, [this]() {
        load();
    });

    load();
}

void SoundFlagSettingsModel::load()
{
    IF_ASSERT_FAILED(m_item && m_item->isSoundFlag()) {
        return;
    }

    initTitle();
    initAvailablePresets();

    emit contextMenuModelChanged();
}

void SoundFlagSettingsModel::initTitle()
{
    const AudioInputParams& params = currentAudioInputParams();

    QString name = muse::audio::audioSourceName(params).toQString();
    QString category = muse::audio::audioSourceCategoryName(params).toQString();
    QString title = category + ": " + name;

    setTitle(title);
}

void SoundFlagSettingsModel::initAvailablePresets()
{
    m_availablePresetsInited = false;

    playbackController()->availableSoundPresets(makeInstrumentTrackId(m_item))
    .onResolve(this, [this](const SoundPresetList& presets) {
        setAvailableSoundPresets(presets);

        m_availablePresetsInited = true;
        emit initedChanged();
    });

    emit selectedPresetCodesChanged();
    emit selectedPlayingTechniqueCodeChanged();
}

void SoundFlagSettingsModel::togglePreset(const QString& presetCode)
{
    if (!m_item) {
        return;
    }

    QStringList newPresetCodes;

    if (playbackConfiguration()->soundPresetsMultiSelectionEnabled()) {
        newPresetCodes = selectedPresetCodes();

        if (newPresetCodes.contains(presetCode)) {
            if (newPresetCodes.size() == 1) {
                return;
            }

            newPresetCodes.removeOne(presetCode);
        } else {
            newPresetCodes.push_back(presetCode);
        }
    } else {
        newPresetCodes = QStringList{ presetCode };
    }

    SoundFlag* soundFlag = toSoundFlag(m_item);

    beginCommand();
    soundFlag->undoChangeSoundFlag(StringList(newPresetCodes), soundFlag->playingTechnique());
    bool needUpdateNotation = updateStaffText();
    endCommand();

    if (currentNotation()->interaction()->isTextEditingStarted()) {
        currentNotation()->interaction()->endEditText();
    }

    if (needUpdateNotation) {
        updateNotation();
    }

    loadAvailablePlayingTechniques();

    emit selectedPresetCodesChanged();
    emit contextMenuModelChanged();
}

void SoundFlagSettingsModel::togglePlayingTechnique(const QString& playingTechniqueCode)
{
    if (!m_item) {
        return;
    }

    SoundFlag* soundFlag = toSoundFlag(m_item);

    beginCommand();
    soundFlag->undoChangeSoundFlag(soundFlag->soundPresets(), String(playingTechniqueCode));
    bool needUpdateNotation = updateStaffText();
    endCommand();

    if (currentNotation()->interaction()->isTextEditingStarted()) {
        currentNotation()->interaction()->endEditText();
    }

    if (needUpdateNotation) {
        updateNotation();
    }

    emit selectedPlayingTechniqueCodeChanged();
    emit contextMenuModelChanged();
}

muse::uicomponents::MenuItem* SoundFlagSettingsModel::buildMenuItem(const QString& actionCode, const TranslatableString& title,
                                                                    bool enabled)
{
    muse::uicomponents::MenuItem* item = new muse::uicomponents::MenuItem(this);
    item->setId(actionCode);

    muse::ui::UiAction action;
    action.code = muse::actions::codeFromQString(actionCode);
    action.title = title;
    item->setAction(action);

    muse::ui::UiActionState state;
    state.enabled = enabled;
    item->setState(state);

    return item;
}

QString SoundFlagSettingsModel::defaultPresetCode() const
{
    return !m_availablePresets.empty() ? m_availablePresets.front().code.toQString() : QString();
}

QString SoundFlagSettingsModel::defaultPlayingTechniqueCode() const
{
    return !m_availablePlayingTechniquesModel.empty()
           ? String(m_availablePlayingTechniquesModel.front().toMap()["code"].toString())
           : String();
}

QVariantList SoundFlagSettingsModel::contextMenuModel()
{
    muse::uicomponents::MenuItemList items;

    SoundFlag* soundFlag = toSoundFlag(m_item);
    if (!soundFlag) {
        return {};
    }

    auto isResetEnabled = [=]() {
        bool enabled = false;

        const SoundFlag::PresetCodes& activePresetCodes = soundFlag->soundPresets();
        const String playingTechnique = soundFlag->playingTechnique();

        if (!activePresetCodes.empty()) {
            enabled = activePresetCodes != StringList { defaultPresetCode() };
        }

        if (!playingTechnique.empty()) {
            enabled |= playingTechnique != defaultPlayingTechniqueCode();
        }

        return enabled;
    };

    muse::uicomponents::MenuItem* resetItem = buildMenuItem(RESET_MENU_ID, TranslatableString("playback", "Reset to default sound"),
                                                            isResetEnabled());

    muse::ui::UiAction resetAction = resetItem->action();
    resetAction.iconCode = muse::ui::IconCode::Code::UNDO;
    resetItem->setAction(resetAction);

    items << resetItem;

    muse::uicomponents::MenuItem* multiSelectionItem
        = buildMenuItem(MULTI_SELECTION_MENU_ID, TranslatableString("playback", "Allow multiple selection"));

    muse::ui::UiAction multiSelectionAction = multiSelectionItem->action();
    multiSelectionAction.checkable = muse::ui::Checkable::Yes;
    multiSelectionItem->setAction(multiSelectionAction);

    muse::ui::UiActionState multiSelectionActionState = multiSelectionItem->state();
    multiSelectionActionState.checked = playbackConfiguration()->soundPresetsMultiSelectionEnabled();
    multiSelectionItem->setState(multiSelectionActionState);

    items << multiSelectionItem;

    return muse::uicomponents::menuItemListToVariantList(items);
}

void SoundFlagSettingsModel::handleContextMenuItem(const QString& menuId)
{
    if (menuId == RESET_MENU_ID) {
        SoundFlag* soundFlag = toSoundFlag(m_item);

        beginCommand();

        soundFlag->undoChangeSoundFlag({ defaultPresetCode() }, defaultPlayingTechniqueCode());
        bool needUpdateNotation = updateStaffText();

        endCommand();

        if (needUpdateNotation) {
            updateNotation();
        }

        emit selectedPresetCodesChanged();
        emit selectedPlayingTechniqueCodeChanged();
    } else if (menuId == MULTI_SELECTION_MENU_ID) {
        playbackConfiguration()->setSoundPresetsMultiSelectionEnabled(!playbackConfiguration()->soundPresetsMultiSelectionEnabled());
        emit contextMenuModelChanged();
    }
}

bool SoundFlagSettingsModel::updateStaffText()
{
    EngravingItem* parent = m_item->parentItem();
    if (!parent || !parent->isStaffText()) {
        return false;
    }

    String newText = mtrc("engraving", "Staff text");
    const SoundFlag* soundFlag = toSoundFlag(m_item);
    const SoundFlag::PresetCodes& activePresetCodes = soundFlag->soundPresets();

    StringList strs;

    for (const SoundPreset& preset : m_availablePresets) {
        if (preset.name.empty()) {
            continue;
        }

        if (mu::contains(activePresetCodes, preset.code)) {
            strs << preset.name;
        }
    }

    const SoundFlag::PlayingTechniqueCode& techniqueCode = soundFlag->playingTechnique();
    if (!techniqueCode.empty()) {
        if (techniqueCode.toStdString() == mu::mpe::ORDINARY_PLAYING_TECHNIQUE_CODE) {
            strs << mtrc("playback", "ordinary");
        } else {
            strs << soundFlag->playingTechnique();
        }
    }

    if (!strs.empty()) {
        newText = strs.join(u", ").toLower();
    }

    StaffText* staffTextItem = toStaffText(parent);
    bool isTextChanged = staffTextItem->xmlText() != newText;

    if (isTextChanged) {
        staffTextItem->undoChangeProperty(Pid::TEXT, newText);
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

const AudioInputParams& SoundFlagSettingsModel::currentAudioInputParams() const
{
    return audioSettings()->trackInputParams(makeInstrumentTrackId(m_item));
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

QRect SoundFlagSettingsModel::iconRect() const
{
    return m_item ? fromLogical(m_item->canvasBoundingRect()).toQRect() : QRect();
}

QVariantList SoundFlagSettingsModel::availablePresets() const
{
    return m_availablePresetsModel;
}

QStringList SoundFlagSettingsModel::selectedPresetCodes() const
{
    if (!m_item) {
        return {};
    }

    QStringList result;
    for (const String& presetCode : toSoundFlag(m_item)->soundPresets()) {
        result << presetCode.toQString();
    }

    return result;
}

QVariantList SoundFlagSettingsModel::availablePlayingTechniques() const
{
    return m_availablePlayingTechniquesModel;
}

void SoundFlagSettingsModel::setAvailableSoundPresets(const SoundPresetList& presets)
{
    if (m_availablePresets == presets) {
        return;
    }

    m_availablePresets = presets;
    m_availablePresetsModel = buildAvailablePresetsModel(presets);
    emit availablePresetsChanged();

    loadAvailablePlayingTechniques();

    emit contextMenuModelChanged();
}

void SoundFlagSettingsModel::loadAvailablePlayingTechniques()
{
    QStringList selectedPresetCodes = this->selectedPresetCodes();
    std::set<String> availablePlayingTechniqueCodes;

    for (const SoundPreset& preset : m_availablePresets) {
        if (selectedPresetCodes.empty()) {
            if (!preset.isDefault) {
                continue;
            }
        } else if (!selectedPresetCodes.contains(preset.code)) {
            continue;
        }

        auto techniqueIt = preset.attributes.find(PLAYING_TECHNIQUES_ATTRIBUTE);
        if (techniqueIt == preset.attributes.end()) {
            continue;
        }

        for (const String& code : techniqueIt->second.split(u"|")) {
            availablePlayingTechniqueCodes << code;
        }
    }

    QVariantList newModel = buildAvailablePlayingTechniquesModel(availablePlayingTechniqueCodes);
    if (m_availablePlayingTechniquesModel == newModel) {
        return;
    }

    m_availablePlayingTechniquesModel = std::move(newModel);
    emit availablePlayingTechniquesChanged();
    emit selectedPlayingTechniqueCodeChanged();
}

QString SoundFlagSettingsModel::selectedPlayingTechniqueCode() const
{
    if (!m_item) {
        return QString();
    }

    QString result = toSoundFlag(m_item)->playingTechnique().toQString();

    return result;
}
