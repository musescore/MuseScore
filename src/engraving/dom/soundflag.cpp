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

using namespace mu::engraving;

static const ElementStyle SOUND_FLAG_STYLE {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

SoundFlag::SoundFlag(Segment* parent, TextStyleType tid)
    : StaffTextBase(ElementType::SOUND_FLAG, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&SOUND_FLAG_STYLE);
}

SoundFlag* SoundFlag::clone() const
{
    return new SoundFlag(*this);
}

PropertyValue SoundFlag::getProperty(Pid id) const
{
    if (id == Pid::SOUND_PRESET) {
        return m_soundPreset;
    }

    return StaffTextBase::getProperty(id);
}

PropertyValue SoundFlag::propertyDefault(Pid id) const
{
    if (id == Pid::SOUND_PRESET) {
        return PropertyValue();
    }

    return StaffTextBase::propertyDefault(id);
}

bool SoundFlag::setProperty(Pid id, const PropertyValue& val)
{
    if (id == Pid::SOUND_PRESET) {
        m_soundPreset = val.value<String>();
        return true;
    }

    return StaffTextBase::setProperty(id, val);
}

const mu::String& SoundFlag::soundPreset() const
{
    return m_soundPreset;
}

void SoundFlag::setSoundPreset(const String& val)
{
    m_soundPreset = val;
}
