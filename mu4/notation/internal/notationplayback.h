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
#ifndef MU_DOMAIN_NOTATIONPLAYBACK_H
#define MU_DOMAIN_NOTATIONPLAYBACK_H

#include "../inotationplayback.h"
#include "igetscore.h"
#include "async/asyncable.h"

namespace Ms {
class Score;
class EventMap;
}

namespace mu {
namespace domain {
namespace notation {
class NotationPlayback : public INotationPlayback, public async::Asyncable
{
public:
    NotationPlayback(IGetScore* getScore);

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

    struct ChanInfo {
        size_t trackIdx = 0;
        uint16_t bank = 0;
        uint16_t program = 0;
        uint16_t channel = 0;
    };

    struct MetaInfo {
        size_t tracksCount = 0;
        std::map<uint16_t, ChanInfo> channels;
    };

    void makeInitData(midi::MidiData& data, Ms::Score* score) const;
    void makeEventMap(Ms::EventMap& eventMap, Ms::Score* score) const;
    void makeInitEvents(std::vector<midi::Event>& events, const Ms::Score* score) const;
    void makeTracks(std::vector<midi::Track>& tracks, const Ms::Score* score) const;
    void makeEvents(midi::Events& events, const Ms::EventMap& msevents) const;

    void makeTempoMap(midi::TempoMap& tempos, const Ms::Score* score) const;
    void makeSynthMap(midi::SynthMap& synthMap, const Ms::Score* score) const;

    int instrumentBank(const Ms::Instrument* inst) const;

    // play element
    midi::MidiData playNoteMidiData(const Ms::Note* note) const;
    midi::MidiData playChordMidiData(const Ms::Chord* chord) const;
    midi::MidiData playHarmonyMidiData(const Ms::Harmony* harmony) const;

    IGetScore* m_getScore = nullptr;
    async::Channel<int> m_playPositionTickChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATIONPLAYBACK_H
