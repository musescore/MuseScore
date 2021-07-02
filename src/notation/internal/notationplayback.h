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
#ifndef MU_NOTATION_NOTATIONPLAYBACK_H
#define MU_NOTATION_NOTATIONPLAYBACK_H

#include <memory>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "../inotationplayback.h"
#include "igetscore.h"
#include "inotationmidievents.h"
#include "inotationparts.h"
#include "inotationconfiguration.h"

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
    NotationPlayback(IGetScore* getScore, async::Notification notationChanged, INotationMidiEventsPtr midiDataProvider);
    ~NotationPlayback();

    void init(INotationPartsPtr parts);

    void load();
    midi::MidiData instrumentMidiData(const InstrumentTrackId& id) const override;
    std::vector<InstrumentTrackId> instrumentTrackIdList() const override;
    async::Channel<InstrumentTrackId> instrumentTrackRemoved() const override;
    async::Channel<InstrumentTrackId> instrumentTrackAdded() const override;

    QTime totalPlayTime() const override;

    float tickToSec(midi::tick_t tick) const override;
    midi::tick_t secToTick(float sec) const override;

    QRect playbackCursorRectByTick(midi::tick_t tick) const override;

    RetVal<midi::tick_t> playPositionTickByElement(const Element* element) const override;

    Ret playElementMidiData(const Element* element) override;

    void addLoopBoundary(LoopBoundaryType boundaryType, midi::tick_t tick) override;
    void setLoopBoundariesVisible(bool visible) override;
    ValCh<LoopBoundaries> loopBoundaries() const override;

    Tempo tempo(midi::tick_t tick) const override;
    MeasureBeat beat(midi::tick_t tick) const override;
    midi::tick_t beatToTick(int measureIndex, int beatIndex) const override;

private:
    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;

    midi::TempoMap makeTempoMap() const;

    int instrumentBank(const Ms::Instrument* instrument) const;

    // play element
    Ret playNoteMidiData(const Ms::Note* note) const;
    Ret playChordMidiData(const Ms::Chord* chord) const;
    Ret playHarmonyMidiData(const Ms::Harmony* harmony) const;

    void addLoopIn(int tick);
    void addLoopOut(int tick);
    QRect loopBoundaryRectByTick(LoopBoundaryType boundaryType, int tick) const;
    void updateLoopBoundaries();

    const Ms::TempoText* tempoText(int tick) const;

    midi::MidiData buildMidiData(const Ms::Part* part) const;
    midi::MidiMapping buildMidiMapping(const Ms::Part* part) const;
    midi::MidiStream buildMidiStream(const Ms::Part* part) const;

    IGetScore* m_getScore = nullptr;
    INotationPartsPtr m_notationParts = nullptr;
    INotationMidiEventsPtr m_midiEventsProvider = nullptr;
    async::Channel<int> m_playPositionTickChanged;
    ValCh<LoopBoundaries> m_loopBoundaries;

    std::unordered_map<std::string, midi::MidiData> m_instrumentsMidiData;
    async::Channel<InstrumentTrackId> m_instrumentTrackRemoved;
    async::Channel<InstrumentTrackId> m_instrumentTrackAdded;
};
}

#endif // MU_NOTATION_NOTATIONPLAYBACK_H
