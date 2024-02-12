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
#include "engraving/dom/stafftext.h"
#include "engraving/dom/soundflag.h"

#include "audio/audioutils.h"

#include "translation.h"

using namespace mu;
using namespace mu::playback;

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

    initSourceType();
    initTitle();

    emit showTextChanged();
    emit textChanged();
}

void SoundFlagSettingsModel::initSourceType()
{
    audio::AudioSourceType type = currentAudioInputParams().type();
    SourceType sourceType = SourceType::Undefined;

    if (audio::AudioSourceType::MuseSampler == type) {
        sourceType = SourceType::MuseSounds;
    }

    setSourceType(sourceType);
}

void SoundFlagSettingsModel::initTitle()
{
    audio::AudioInputParams params = currentAudioInputParams();
    audio::AudioSourceType type = params.type();

    QString title;
    QString name = audio::audioSourceName(params).toQString();

    if (audio::AudioSourceType::MuseSampler == type) {
        title = qtrc("playback", "Muse Sounds:") + " " + name;
    }

    setTitle(title);
}

engraving::StaffText* SoundFlagSettingsModel::staffText() const
{
    engraving::EngravingItem* parent = m_item->parentItem();
    return parent && parent->isStaffText() ? engraving::toStaffText(parent) : nullptr;
}

project::IProjectAudioSettingsPtr SoundFlagSettingsModel::audioSettings() const
{
    IF_ASSERT_FAILED(globalContext()->currentProject()) {
        return nullptr;
    }

    return globalContext()->currentProject()->audioSettings();
}

audio::AudioInputParams SoundFlagSettingsModel::currentAudioInputParams() const
{
    engraving::Part* part = m_item->part();
    IF_ASSERT_FAILED(part) {
        return {};
    }

    engraving::Instrument* instrument = part->instrument(m_item->tick());
    IF_ASSERT_FAILED(instrument) {
        return {};
    }

    return audioSettings()->trackInputParams({ part->id(), instrument->id().toStdString() });
}

SoundFlagSettingsModel::SourceType SoundFlagSettingsModel::sourceType() const
{
    return m_sourceType;
}

void SoundFlagSettingsModel::setSourceType(SourceType type)
{
    if (m_sourceType == type) {
        return;
    }

    m_sourceType = type;
    emit sourceTypeChanged();
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
