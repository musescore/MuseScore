//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "abstractaudiosource.h"

using namespace mu::audio;

AbstractAudioSource::~AbstractAudioSource() = default;

void AbstractAudioSource::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
}

mu::async::Channel<unsigned int> AbstractAudioSource::streamsCountChanged() const
{
    return m_streamsCountChanged;
}

const float* AbstractAudioSource::data() const
{
    return m_buffer.data();
}

void AbstractAudioSource::setBufferSize(unsigned int samples)
{
    auto sc = streamCount();
    auto targetSize = samples * sc;
    if (targetSize > 0 && m_buffer.size() < targetSize) {
        m_buffer.resize(samples * streamCount());
    }
}
