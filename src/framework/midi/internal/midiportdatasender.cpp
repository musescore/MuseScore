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
    LOGD() << "chunk.beginTick: " << chunk.beginTick;
    m_midiData.chunks.insert({ chunk.beginTick, chunk });
}

bool MidiPortDataSender::sendEvents(tick_t fromTick, tick_t toTick)
{
    static const std::set<EventType> SKIP_EVENTS = { EventType::ME_EOT, EventType::ME_TICK1, EventType::ME_TICK2 };

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

        if (SKIP_EVENTS.find(event.type()) == SKIP_EVENTS.end()) {
            midiOutPort()->sendEvent(event);
        }

        ++pos;
    }

    return true;
}
