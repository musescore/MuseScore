/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_NOTATION_NOTATIONPLAYBACK_H
#define MU_NOTATION_NOTATIONPLAYBACK_H

#include <memory>

#include "../inotationplayback.h"
#include "igetscore.h"
#include "async/asyncable.h"
#include "inotationconfiguration.h"

#include "modularity/ioc.h"

namespace Ms {
class Score;
class EventMap;
class MidiRenderer;
}

namespace mu::notation {
class NotationPlayback : public INotationPlayback, public async::Asyncable
{
    INJECT(notation, INotationConfiguration, configuration)

public:
    NotationPlayback(IGetScore* getScore, async::Notification notationChanged);

    void init();

    std::shared_ptr<midi::MidiStream> midiStream() const override;

    QTime totalPlayTime() const override;

    float tickToSec(int tick) const override;
    int secToTick(float sec) const override;

    QRect playbackCursorRectByTick(int tick) const override;

    RetVal<int> playPositionTick() const override;
    void setPlayPositionTick(int tick) override;
    bool setPlayPositionByElement(const Element* element) override;
    async::Channel<int> playPositionTickChanged() const override;

    midi::MidiData playElementMidiData(const Element* element) const override;

    void addLoopBoundary(LoopBoundaryType boundaryType, int tick) override;
    void setLoopBoundariesVisible(bool visible) override;
    ValCh<LoopBoundaries> loopBoundaries() const override;

    Tempo tempo(int tick) const override;
    MeasureBeat beat(int tick) const override;
    int beatToTick(int measureIndex, int beatIndex) const override;

private:
    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;

    void makeInitData(midi::MidiData& data, Ms::Score* score) const;
    void makeInitEvents(std::vector<midi::Event>& events, const Ms::Score* score) const;
    void makeTracks(std::vector<midi::Track>& tracks, const Ms::Score* score) const;
    void makeTempoMap(midi::TempoMap& tempos, const Ms::Score* score) const;
    void makeSynthMap(midi::SynthMap& synthMap, const Ms::Score* score) const;

    void onChunkRequest(midi::tick_t tick);
    void makeChunk(midi::Chunk& chunk, midi::tick_t fromTick) const;

    int instrumentBank(const Ms::Instrument* instrument) const;

    // play element
    midi::MidiData playNoteMidiData(const Ms::Note* note) const;
    midi::MidiData playChordMidiData(const Ms::Chord* chord) const;
    midi::MidiData playHarmonyMidiData(const Ms::Harmony* harmony) const;

    void addLoopIn(int tick);
    void addLoopOut(int tick);
    QRect loopBoundaryRectByTick(LoopBoundaryType boundaryType, int tick) const;
    void updateLoopBoundaries();

    const Ms::TempoText* tempoText(int tick) const;

    IGetScore* m_getScore = nullptr;
    std::shared_ptr<midi::MidiStream> m_midiStream;
    std::unique_ptr<Ms::MidiRenderer> m_midiRenderer;
    async::Channel<int> m_playPositionTickChanged;
    ValCh<LoopBoundaries> m_loopBoundaries;
};
}

#endif // MU_NOTATION_NOTATIONPLAYBACK_H
