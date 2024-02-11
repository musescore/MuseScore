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

#include "vstparamsmodel.h"

#include "engraving/dom/utils.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/soundflag.h"

#include "audio/audiotypes.h"

#include "translation.h"

using namespace mu;
using namespace mu::playback;

static constexpr int INVALID_KEY = -1;

static const String KEYSWITCH_USE_FLAT_PARAM_CODE("keyswitchUseFlat");

VSTParamsModel::VSTParamsModel(QObject* parent)
    : QObject(parent)
{
}

void VSTParamsModel::init()
{
    if (!selection()) {
        return;
    }

    engraving::EngravingItem* selectedItem = selection()->element();
    if (!selectedItem) {
        return;
    }

    if (selectedItem->isStaffText()) {
        m_item = toStaffText(selectedItem)->soundFlag();
        IF_ASSERT_FAILED(m_item) {
            return;
        }
    } else if (selectedItem->isSoundFlag()) {
        m_item = selectedItem;
    }

    engraving::SoundFlag* soundFlag = engraving::toSoundFlag(m_item);
    const engraving::SoundFlag::Params& params = soundFlag->params();

    m_useFlat = mu::contains(params, KEYSWITCH_USE_FLAT_PARAM_CODE)
                && params.at(KEYSWITCH_USE_FLAT_PARAM_CODE).toBool();

    emit keySwitchChanged();
    emit keySwitchStrChanged();
}

QString VSTParamsModel::increaseKeySwitch(const QString& keyStr)
{
    if (keyStr.isEmpty()) {
        return defaultKeySwitchStr();
    }

    String value = engraving::convertPitchStringFlatsAndSharpsToUnicode(keyStr);
    return engraving::pitch2string(engraving::string2pitch(value) + 1, false /* useFlats */);
}

QString VSTParamsModel::decreaseKeySwitch(const QString& keyStr)
{
    if (keyStr.isEmpty()) {
        return defaultKeySwitchStr();
    }

    String value = engraving::convertPitchStringFlatsAndSharpsToUnicode(keyStr);
    return engraving::pitch2string(engraving::string2pitch(value) - 1, true /* useFlats */);
}

QString VSTParamsModel::defaultKeySwitchStr() const
{
    return "C0";
}

int VSTParamsModel::keySwitch() const
{
    if (!m_item) {
        return INVALID_KEY;
    }

    engraving::SoundFlag* soundFlag = engraving::toSoundFlag(m_item);
    const engraving::SoundFlag::Params& params = soundFlag->params();
    if (!mu::contains(params, String(audio::KEYSWITCH_PARAM_CODE))) {
        return INVALID_KEY;
    }

    return params.at(audio::KEYSWITCH_PARAM_CODE).toInt();
}

QString VSTParamsModel::keySwitchStr() const
{
    int key = keySwitch();
    if (key == INVALID_KEY) {
        return QString();
    }

    return engraving::pitch2string(key, m_useFlat).toUpper();
}

void VSTParamsModel::setKeySwitchStr(const QString& str)
{
    if (keySwitch() == str) {
        return;
    }

    engraving::SoundFlag* soundFlag = engraving::toSoundFlag(m_item);
    engraving::SoundFlag::Params params = soundFlag->params();

    String value = engraving::convertPitchStringFlatsAndSharpsToUnicode(str);
    params[audio::KEYSWITCH_PARAM_CODE] = Val(engraving::string2pitch(value));

    m_useFlat = value.contains(u'♭');
    params[KEYSWITCH_USE_FLAT_PARAM_CODE] = Val(m_useFlat);

    undoStack()->prepareChanges();
    soundFlag->undoChangeSoundFlag(soundFlag->soundPresets(), params);
    undoStack()->commitChanges();

    emit keySwitchChanged();
    emit keySwitchStrChanged();
}

notation::INotationSelectionPtr VSTParamsModel::selection() const
{
    IF_ASSERT_FAILED(globalContext()->currentNotation()) {
        return nullptr;
    }

    return globalContext()->currentNotation()->interaction()->selection();
}

notation::INotationUndoStackPtr VSTParamsModel::undoStack() const
{
    IF_ASSERT_FAILED(globalContext()->currentNotation()) {
        return nullptr;
    }

    return globalContext()->currentNotation()->undoStack();
}
