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

#include "soundflag.h"

#include "undo.h"

using namespace mu::engraving;

static const ElementStyle SOUND_FLAG_STYLE {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

SoundFlag::SoundFlag(Segment* parent)
    : TextBase(ElementType::SOUND_FLAG, parent, TextStyleType::DEFAULT, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&SOUND_FLAG_STYLE);
}

SoundFlag* SoundFlag::clone() const
{
    return new SoundFlag(*this);
}

const SoundFlag::PresetCodes& SoundFlag::soundPresets() const
{
    return m_soundPresets;
}

void SoundFlag::setSoundPresets(const PresetCodes& soundPresets)
{
    m_soundPresets = soundPresets;
}

const SoundFlag::Params& SoundFlag::params() const
{
    return m_params;
}

void SoundFlag::setParams(const Params& params)
{
    m_params = params;
}

void SoundFlag::undoChangeSoundFlag(const PresetCodes& presets, const Params& params)
{
    score()->undo(new ChangeSoundFlag(this, presets, params));
}
