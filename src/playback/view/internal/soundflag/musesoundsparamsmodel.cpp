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

#include "musesoundsparamsmodel.h"

#include "engraving/dom/soundflag.h"

#include "audio/audiotypes.h"

using namespace mu;
using namespace mu::playback;

MuseSoundsParamsModel::MuseSoundsParamsModel(QObject* parent)
    : QObject(parent)
{
}

void MuseSoundsParamsModel::init()
{
    if (!selection()) {
        return;
    }

    m_item = selection()->element();
    if (!m_item) {
        return;
    }

    engraving::Part* part = m_item->part();
    engraving::Instrument* instrument = part->instrument(m_item->tick());

    playbackController()->availableSoundPresets({ part->id(), instrument->id().toStdString() })
    .onResolve(this, [this](audio::SoundPresetList presets) {
        setAvailablePresets(presets);
    });

    emit presetCodesChanged();
}

void MuseSoundsParamsModel::togglePreset(const QString& presetCode, bool forceMultiSelection)
{
    if (!m_item) {
        return;
    }

    String presetCodeStr = String::fromQString(presetCode);

    engraving::SoundFlag* soundFlag = engraving::toSoundFlag(m_item);
    engraving::SoundFlag::PresetCodes presetCodes = soundFlag->soundPresets();

    if (forceMultiSelection || playbackConfiguration()->isSoundFlagsMultiSelectionEnabled()) {
        if (presetCodes.contains(presetCode)) {
            if (presetCodes.size() == 1) {
                return;
            }

            presetCodes.removeAll(presetCodeStr);
        } else {
            presetCodes.emplace_back(presetCodeStr);
        }
    } else {
        presetCodes = { presetCodeStr };
    }

    undoStack()->prepareChanges();
    soundFlag->undoChangeSoundFlag(presetCodes, soundFlag->params(), soundFlag->isTextVisible());
    undoStack()->commitChanges();

    emit presetCodesChanged();
}

QVariantList MuseSoundsParamsModel::availablePresets() const
{
    return m_availablePresets;
}

void MuseSoundsParamsModel::setAvailablePresets(const audio::SoundPresetList& presets)
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

QStringList MuseSoundsParamsModel::presetCodes() const
{
    if (!m_item) {
        return {};
    }

    return engraving::toSoundFlag(m_item)->soundPresets().toQStringList();
}

notation::INotationSelectionPtr MuseSoundsParamsModel::selection() const
{
    IF_ASSERT_FAILED(globalContext()->currentNotation()) {
        return nullptr;
    }

    return globalContext()->currentNotation()->interaction()->selection();
}

notation::INotationUndoStackPtr MuseSoundsParamsModel::undoStack() const
{
    IF_ASSERT_FAILED(globalContext()->currentNotation()) {
        return nullptr;
    }

    return globalContext()->currentNotation()->undoStack();
}
