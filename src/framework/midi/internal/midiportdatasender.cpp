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
#include "midiportdatasender.h"

#include "log.h"

using namespace mu::midi;

void MidiPortDataSender::setMidiStream(std::shared_ptr<MidiStream> stream)
{
    m_stream = stream;
    m_midiData = m_stream->initData;

    for (const Event& e : m_midiData.initEvents) {
        midiOutPort()->sendEvent(e);
    }
}

void MidiPortDataSender::onChunkReceived(const Chunk& chunk)
{
    m_midiData.chunks.insert({ chunk.beginTick, chunk });
}

bool MidiPortDataSender::sendEvents(tick_t fromTick, tick_t toTick)
{
    //! NOTE Here we need to set up a callback to receive data in the same thread as reading,
    //! and accordingly, then the mutex is not needed
    if (m_stream->isStreamingAllowed && !m_isStreamConnected) {
        //! NOTE Requests are made in the sequencer, here we only listen and receive data (in sync with the sequencer)
        m_stream->stream.onReceive(this, [this](const Chunk& chunk) { onChunkReceived(chunk); });
        m_isStreamConnected = true;
    }

    if (m_midiData.chunks.empty()) {
        return false;
    }

    auto chunkIt = m_midiData.chunks.upper_bound(fromTick);
    --chunkIt;

    const Chunk& chunk = chunkIt->second;
    auto pos = chunk.events.lower_bound(fromTick);

    while (1) {
        const Chunk& curChunk = chunkIt->second;
        if (pos == curChunk.events.end()) {
            ++chunkIt;
            if (chunkIt == m_midiData.chunks.end()) {
                break;
            }

            const Chunk& nextChunk = chunkIt->second;
            if (nextChunk.events.empty()) {
                break;
            }

            pos = nextChunk.events.begin();
        }

        if (pos->first >= toTick) {
            break;
        }

        const Event& event = pos->second;
        if (event) {
            midiOutPort()->sendEvent(event);
        }

        ++pos;
    }

    return true;
}

bool MidiPortDataSender::sendSingleEvent(const Event& event)
{
    return midiOutPort()->sendEvent(event);
}
