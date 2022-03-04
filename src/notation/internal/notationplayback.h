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
#include "engraving/playback/playbackmodel.h"

#include "../inotationplayback.h"
#include "igetscore.h"
#include "inotationundostack.h"
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
    NotationPlayback(IGetScore* getScore, async::Notification notationChanged);

    void init(INotationUndoStackPtr undoStack) override;

    const engraving::InstrumentTrackId& metronomeTrackId() const override;
    const mpe::PlaybackData& trackPlaybackData(const engraving::InstrumentTrackId& trackId) const override;
    void triggerEventsForItem(const EngravingItem* item) override;

    audio::msecs_t totalPlayTime() const override;
    async::Channel<audio::msecs_t> totalPlayTimeChanged() const override;

    float playedTickToSec(midi::tick_t tick) const override;
    midi::tick_t secToPlayedtick(float sec) const override;
    midi::tick_t secToTick(float sec) const override;

    RectF playbackCursorRectByTick(midi::tick_t tick) const override;

    RetVal<midi::tick_t> playPositionTickByElement(const EngravingItem* element) const override;

    void addLoopBoundary(LoopBoundaryType boundaryType, midi::tick_t tick) override;
    void setLoopBoundariesVisible(bool visible) override;
    ValCh<LoopBoundaries> loopBoundaries() const override;

    Tempo tempo(midi::tick_t tick) const override;
    MeasureBeat beat(midi::tick_t tick) const override;
    midi::tick_t beatToTick(int measureIndex, int beatIndex) const override;

private:
    Ms::Score* score() const;

    void addLoopIn(int tick);
    void addLoopOut(int tick);
    RectF loopBoundaryRectByTick(LoopBoundaryType boundaryType, int tick) const;
    void updateLoopBoundaries();
    void updateTotalPlayTime();

    const Ms::TempoText* tempoText(int tick) const;

    IGetScore* m_getScore = nullptr;
    async::Channel<int> m_playPositionTickChanged;
    ValCh<LoopBoundaries> m_loopBoundaries;

    audio::msecs_t m_totalPlayTime = 0;
    async::Channel<audio::msecs_t> m_totalPlayTimeChanged;

    mutable engraving::PlaybackModel m_playbackModel;
};
}

#endif // MU_NOTATION_NOTATIONPLAYBACK_H
