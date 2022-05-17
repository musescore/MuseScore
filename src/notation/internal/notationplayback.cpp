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
#include "notationplayback.h"

#include <cmath>

#include "log.h"

#include "libmscore/rendermidi.h"
#include "libmscore/masterscore.h"
#include "libmscore/tempo.h"
#include "libmscore/part.h"
#include "libmscore/instrument.h"
#include "libmscore/repeatlist.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/system.h"
#include "libmscore/scorefont.h"
#include "libmscore/page.h"
#include "libmscore/staff.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/harmony.h"
#include "libmscore/tempotext.h"
#include "libmscore/tempo.h"

#include "notationerrors.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::midi;
using namespace mu::async;
using namespace mu::engraving;

NotationPlayback::NotationPlayback(IGetScore* getScore,
                                   async::Notification notationChanged)
    : m_getScore(getScore)
{
    notationChanged.onNotify(this, [this]() {
        updateLoopBoundaries();
    });
}

Ms::Score* NotationPlayback::score() const
{
    return m_getScore->score();
}

void NotationPlayback::init(INotationUndoStackPtr undoStack)
{
    IF_ASSERT_FAILED(score() && undoStack) {
        return;
    }

    m_playbackModel.setPlayRepeats(configuration()->isPlayRepeatsEnabled());
    m_playbackModel.load(score());

    updateTotalPlayTime();
    m_playbackModel.dataChanged().onNotify(this, [this]() {
        updateTotalPlayTime();
    });

    configuration()->isPlayRepeatsChanged().onNotify(this, [this]() {
        bool expandRepeats = configuration()->isPlayRepeatsEnabled();
        if (expandRepeats != m_playbackModel.isPlayRepeatsEnabled()) {
            m_playbackModel.setPlayRepeats(expandRepeats);
            m_playbackModel.reload();
        }
    });

    score()->posChanged().onReceive(this, [this](Ms::POS pos, int tick) {
        if (Ms::POS::CURRENT == pos) {
            m_playPositionTickChanged.send(tick);
        } else {
            updateLoopBoundaries();
        }
    });
}

const engraving::InstrumentTrackId& NotationPlayback::metronomeTrackId() const
{
    return m_playbackModel.metronomeTrackId();
}

const mpe::PlaybackData& NotationPlayback::trackPlaybackData(const engraving::InstrumentTrackId& trackId) const
{
    return m_playbackModel.resolveTrackPlaybackData(trackId);
}

void NotationPlayback::triggerEventsForItem(const EngravingItem* item)
{
    m_playbackModel.triggerEventsForItem(item);
}

async::Channel<InstrumentTrackId> NotationPlayback::trackAdded() const
{
    return m_playbackModel.trackAdded();
}

async::Channel<InstrumentTrackId> NotationPlayback::trackRemoved() const
{
    return m_playbackModel.trackRemoved();
}

void NotationPlayback::updateLoopBoundaries()
{
    LoopBoundaries boundaries;
    boundaries.loopInTick = score()->loopInTick().ticks();
    boundaries.loopOutTick = score()->loopOutTick().ticks();

    if (boundaries.isNull()) {
        return;
    }

    boundaries.visible = m_loopBoundaries.val.visible;

    if (m_loopBoundaries.val != boundaries) {
        m_loopBoundaries.set(boundaries);
    }
}

void NotationPlayback::updateTotalPlayTime()
{
    Ms::Score* score = m_getScore->score();
    if (!score) {
        return;
    }

    int lastTick = score->repeatList().ticks();
    qreal secs = score->utick2utime(lastTick);

    audio::msecs_t newPlayTime = secs * 1000.f;

    if (m_totalPlayTime == newPlayTime) {
        return;
    }

    m_totalPlayTime = newPlayTime;
    m_totalPlayTimeChanged.send(m_totalPlayTime);
}

audio::msecs_t NotationPlayback::totalPlayTime() const
{
    return m_totalPlayTime;
}

async::Channel<audio::msecs_t> NotationPlayback::totalPlayTimeChanged() const
{
    return m_totalPlayTimeChanged;
}

float NotationPlayback::playedTickToSec(tick_t tick) const
{
    return score() ? score()->utick2utime(tick) : 0.0;
}

tick_t NotationPlayback::secToPlayedTick(float sec) const
{
    if (!score()) {
        return 0;
    }

    return score()->utime2utick(sec);
}

tick_t NotationPlayback::secToTick(float sec) const
{
    if (!score()) {
        return 0;
    }

    tick_t utick = secToPlayedTick(sec);

    return score()->repeatList().utick2tick(utick);
}

RetVal<midi::tick_t> NotationPlayback::playPositionTickByElement(const EngravingItem* element) const
{
    RetVal<tick_t> result;
    result.ret = make_ret(Err::Undefined);
    result.val = 0;

    IF_ASSERT_FAILED(element) {
        return result;
    }

    if (!score()) {
        return result;
    }

    //! NOTE Copied from void ScoreView::mousePressEvent(QMouseEvent* ev)  case ViewState::PLAY: {
    if (!(element->isPlayable() || element->isRest())) {
        return result;
    }

    if (element->isNote()) {
        element = element->parentItem();
    }

    const Ms::ChordRest* cr = Ms::toChordRest(element);

    int ticks = score()->repeatList().tick2utick(cr->tick().ticks());

    return result.make_ok(std::move(ticks));
}

void NotationPlayback::addLoopBoundary(LoopBoundaryType boundaryType, tick_t tick)
{
    if (tick == BoundaryTick::FirstScoreTick) {
        tick = score()->firstMeasure()->tick().ticks();
    } else if (tick == BoundaryTick::LastScoreTick) {
        tick = score()->lastMeasure()->endTick().ticks();
    }

    switch (boundaryType) {
    case LoopBoundaryType::LoopIn:
        addLoopIn(tick);
        break;
    case LoopBoundaryType::LoopOut:
        addLoopOut(tick);
        break;
    case LoopBoundaryType::Unknown:
        break;
    }
}

void NotationPlayback::addLoopIn(int _tick)
{
    Fraction tick = Fraction::fromTicks(_tick);

    if (_tick == BoundaryTick::SelectedNoteTick) {
        tick = score()->pos();
    }

    if (tick >= score()->loopOutTick()) { // If In pos >= Out pos, reset Out pos to end of score
        score()->setLoopOutTick(score()->lastMeasure()->endTick());
    }

    score()->setLoopInTick(tick);
}

void NotationPlayback::addLoopOut(int _tick)
{
    Fraction tick = Fraction::fromTicks(_tick);

    if (_tick == BoundaryTick::SelectedNoteTick) {
        tick = score()->pos() + score()->inputState().ticks();
    }

    if (tick <= score()->loopInTick()) { // If Out pos <= In pos, reset In pos to beginning of score
        score()->setLoopInTick(Fraction(0, 1));
    } else {
        if (tick > score()->lastMeasure()->endTick()) {
            tick = score()->lastMeasure()->endTick();
        }
    }

    score()->setLoopOutTick(tick);
}

void NotationPlayback::setLoopBoundariesVisible(bool visible)
{
    if (m_loopBoundaries.val.visible == visible) {
        return;
    }

    m_loopBoundaries.val.visible = visible;
    m_loopBoundaries.set(m_loopBoundaries.val);
}

mu::ValCh<LoopBoundaries> NotationPlayback::loopBoundaries() const
{
    return m_loopBoundaries;
}

const Tempo& NotationPlayback::tempo(tick_t tick) const
{
    if (!score()) {
        static Tempo empty;
        return empty;
    }

    m_currentTempo.valueBpm = static_cast<int>(score()->tempomap()->tempo(tick).toBPM().val);

    return m_currentTempo;
}

const Ms::TempoText* NotationPlayback::tempoText(int _tick) const
{
    Fraction tick = Fraction::fromTicks(_tick);
    Ms::TempoText* result = nullptr;

    Ms::SegmentType segmentType = Ms::SegmentType::All;
    for (const Ms::Segment* segment = score()->firstSegment(segmentType); segment; segment = segment->next1(segmentType)) {
        for (Ms::EngravingItem* element: segment->annotations()) {
            if (element && element->isTempoText() && element->tick() <= tick) {
                result = Ms::toTempoText(element);
            }
        }
    }

    return result;
}

MeasureBeat NotationPlayback::beat(tick_t tick) const
{
    MeasureBeat measureBeat;

    if (score() && score()->checkHasMeasures()) {
        int dummy = 0;
        score()->sigmap()->tickValues(tick, &measureBeat.measureIndex, &measureBeat.beatIndex, &dummy);

        measureBeat.maxMeasureIndex = score()->measures()->size() - 1;
        measureBeat.maxBeatIndex = score()->sigmap()->timesig(Fraction::fromTicks(tick)).timesig().numerator() - 1;
    }

    return measureBeat;
}

tick_t NotationPlayback::beatToTick(int measureIndex, int beatIndex) const
{
    return score() ? score()->sigmap()->bar2tick(measureIndex, beatIndex) : 0;
}
