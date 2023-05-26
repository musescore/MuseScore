/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "abstractaudiosource.h"

using namespace mu::audio;

void AbstractAudioSource::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
}

bool AbstractAudioSource::isActive() const
{
    return m_isActive;
}

void AbstractAudioSource::setIsActive(bool isActive)
{
    m_isActive = isActive;
}

bool AbstractAudioSource::setAudioChannelsCount(unsigned int /*channels*/)
{
    // Default implementation does not allow changing channel count
    return false;
}
