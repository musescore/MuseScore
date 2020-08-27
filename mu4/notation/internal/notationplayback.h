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
#ifndef MU_NOTATION_NOTATIONPLAYBACK_H
#define MU_NOTATION_NOTATIONPLAYBACK_H

#include <memory>

#include "../inotationplayback.h"
#include "igetscore.h"
#include "async/asyncable.h"

namespace Ms {
class Score;
class EventMap;
class MidiRenderer;
}

namespace mu {
namespace notation {
class NotationPlayback : public INotationPlayback, public async::Asyncable
{
public:
    NotationPlayback(IGetScore* getScore);
    ~NotationPlayback();

    void init();

    std::shared_ptr<midi::MidiStream> midiStream() const override;

    float tickToSec(int tick) const override;
    int secToTick(float sec) const override;

    QRect playbackCursorRectByTick(int tick) const override;

    RetVal<int> playPositionTick() const override;
    void setPlayPositionTick(int tick) override;
    bool setPlayPositionByElement(const Element* e) override;
    async::Channel<int> playPositionTickChanged() const override;

    midi::MidiData playElementMidiData(const Element* e) const override;

private:

    void makeInitData(midi::MidiData& data, Ms::Score* score) const;
    void makeInitEvents(std::vector<midi::Event>& events, const Ms::Score* score) const;
    void makeTracks(std::vector<midi::Track>& tracks, const Ms::Score* score) const;
    void makeTempoMap(midi::TempoMap& tempos, const Ms::Score* score) const;
    void makeSynthMap(midi::SynthMap& synthMap, const Ms::Score* score) const;

    void onChunkRequest(midi::tick_t tick);
    void makeChunk(midi::Chunk& chunk, midi::tick_t fromTick) const;

    int instrumentBank(const Ms::Instrument* inst) const;

    // play element
    midi::MidiData playNoteMidiData(const Ms::Note* note) const;
    midi::MidiData playChordMidiData(const Ms::Chord* chord) const;
    midi::MidiData playHarmonyMidiData(const Ms::Harmony* harmony) const;

    IGetScore* m_getScore = nullptr;
    std::shared_ptr<midi::MidiStream> m_midiStream;
    std::unique_ptr<Ms::MidiRenderer> m_midiRenderer;
    async::Channel<int> m_playPositionTickChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONPLAYBACK_H
