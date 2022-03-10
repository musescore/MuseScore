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

    QObject::connect(score(), &Ms::Score::posChanged, [this](Ms::POS pos, int tick) {
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

void NotationPlayback::updateLoopBoundaries()
{
    LoopBoundaries boundaries;
    boundaries.loopInTick = score()->loopInTick().ticks();
    boundaries.loopOutTick = score()->loopOutTick().ticks();

    if (boundaries.isNull()) {
        return;
    }

    boundaries.loopInRect = loopBoundaryRectByTick(LoopBoundaryType::LoopIn, boundaries.loopInTick);
    boundaries.loopOutRect = loopBoundaryRectByTick(LoopBoundaryType::LoopOut, boundaries.loopOutTick);
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

tick_t NotationPlayback::secToPlayedtick(float sec) const
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

    tick_t utick = secToPlayedtick(sec);

    return score()->repeatList().utick2tick(utick);
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
RectF NotationPlayback::playbackCursorRectByTick(tick_t _tick) const
{
    if (!score()) {
        return {};
    }

    Fraction tick = Fraction::fromTicks(_tick);

    Measure* measure = score()->tick2measureMM(tick);
    if (!measure) {
        return {};
    }

    Ms::System* system = measure->system();
    if (!system) {
        return {};
    }

    qreal x = 0.0;
    Ms::Segment* s = nullptr;
    for (s = measure->first(Ms::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2;
        Fraction t2;
        Ms::Segment* ns = s->next(Ms::SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(Ms::SegmentType::ChordRest);
        }
        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            // measure->width is not good enough because of courtesy keysig, timesig
            Ms::Segment* seg = measure->findSegment(Ms::SegmentType::EndBarLine, measure->tick() + measure->ticks());
            if (seg) {
                x2 = seg->canvasPos().x();
            } else {
                x2 = measure->canvasPos().x() + measure->width();             //safety, should not happen
            }
        }
        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    if (!s) {
        return {};
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score()->spatium();

    qreal mag = _spatium / Ms::SPATIUM20;
    double w  = _spatium * 2.0 + score()->scoreFont()->width(Ms::SymId::noteheadBlack, mag);
    double h  = 6 * _spatium;
    //
    // set cursor height for whole system
    //
    double y2 = 0.0;

    for (int i = 0; i < score()->nstaves(); ++i) {
        Ms::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score()->staff(i)->show()) {
            continue;
        }
        y2 = ss->bbox().bottom();
    }
    h += y2;
    x -= _spatium;
    y -= 3 * _spatium;

    return RectF(x, y, w, h);
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

RectF NotationPlayback::loopBoundaryRectByTick(LoopBoundaryType boundaryType, int _tick) const
{
    Fraction tick = Fraction::fromTicks(_tick);

    // set mark height for whole system
    if (boundaryType == LoopBoundaryType::LoopOut && tick > Fraction(0, 1)) {
        tick -= Fraction::fromTicks(1);
    }

    Measure* measure = score()->tick2measureMM(tick);
    if (measure == nullptr) {
        return RectF();
    }

    qreal x = 0.0;
    const Fraction offset = { 0, 1 };

    Ms::Segment* s = nullptr;
    for (s = measure->first(Ms::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2 = 0;
        Fraction t2;
        Ms::Segment* ns = s->next(Ms::SegmentType::ChordRest);
        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            x2 = measure->canvasPos().x() + measure->width();
        }
        t1 += offset;
        t2 += offset;
        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    if (s == nullptr) {
        return RectF();
    }

    Ms::System* system = measure->system();
    if (system == nullptr || system->page() == nullptr || system->staves()->empty()) {
        return RectF();
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score()->spatium();

    qreal mag = _spatium / Ms::SPATIUM20;
    double width = (_spatium * 2.0 + score()->scoreFont()->width(Ms::SymId::noteheadBlack, mag)) / 3;
    double height = 6 * _spatium;

    // set cursor height for whole system
    double y2 = 0.0;

    for (int i = 0; i < score()->nstaves(); ++i) {
        Ms::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score()->staff(i)->show()) {
            continue;
        }
        y2 = ss->y() + ss->bbox().height();
    }
    height += y2;
    y -= 3 * _spatium;

    if (boundaryType == LoopBoundaryType::LoopIn) {
        x = x - _spatium + width / 1.5;
    } else {
        x = x - _spatium * .5;
    }

    return RectF(x, y, width, height);
}

mu::ValCh<LoopBoundaries> NotationPlayback::loopBoundaries() const
{
    return m_loopBoundaries;
}

Tempo NotationPlayback::tempo(tick_t tick) const
{
    Tempo tempo;

    if (!score()) {
        return tempo;
    }

    const Ms::TempoText* tempoText = this->tempoText(tick);
    Ms::TDuration duration = tempoText ? tempoText->duration() : Ms::TDuration();

    if (!tempoText || !duration.isValid()) {
        tempo.duration = DurationType::V_QUARTER;
        tempo.valueBpm = std::round(score()->tempo(Fraction::fromTicks(tick)).toBPM().val);
        return tempo;
    }

    tempo.valueBpm = tempoText->tempoBpm();
    tempo.duration = duration.type();
    tempo.withDot = duration.dots() > 0;

    return tempo;
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
