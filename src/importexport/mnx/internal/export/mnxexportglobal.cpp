/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "mnxexporter.h"

#include <optional>
#include <utility>

#include "engraving/dom/barline.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/key.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurebase.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/timesig.h"
#include "log.h"
#include "realfn.h"
#include "internal/shared/mnxtypesconv.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
namespace {
class MeasureNumberState
{
public:
    MeasureNumberState() { init(); }

    int displayNumber(const Measure* measure)
    {
        updateForMeasure(measure);
        return m_displayNumber;
    }

private:
    void init()
    {
        m_measureNumber = 1;
        m_measureNumberOffset = 0;
        m_displayNumber = 1;
    }

    void updateForMeasure(const Measure* measure)
    {
        const MeasureBase* previousMB = measure->prev();
        if (previousMB) {
            previousMB = previousMB->mbWithPrecedingSectionBreak();
        }
        if (previousMB) {
            const LayoutBreak* layoutBreak = previousMB->sectionBreakElement();
            if (layoutBreak && layoutBreak->startWithMeasureOne()) {
                init();
            }
        }

        m_measureNumberOffset = measure->measureNumberOffset();
        m_measureNumber += m_measureNumberOffset;

        if (measure->isAnacrusis()) {
            m_displayNumber = 0;
            return;
        }

        m_displayNumber = m_measureNumber++;
    }

    int m_measureNumber = 1;
    int m_measureNumberOffset = 0;
    int m_displayNumber = 1;
};
} // namespace

//---------------------------------------------------------
//   assignTimeSignature
//---------------------------------------------------------

/// Finds the time signature element that this measure introduces, if any. The
/// measure's own timesig() is only a fraction, so this is the only source for the glyph.
static const TimeSig* findTimeSigElement(const Measure* measure)
{
    const Segment* segment = measure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
    if (!segment) {
        return nullptr;
    }
    for (const EngravingItem* item : segment->elist()) {
        if (item && item->isTimeSig()) {
            return toTimeSig(item);
        }
    }
    return nullptr;
}

static void assignTimeSignature(mnx::global::Measure& mnxMeasure, const Measure* measure,
                                std::optional<Fraction>& prevTimeSig,
                                std::optional<mnx::TimeSignatureDisplay>& prevDisplay)
{
    const Fraction timeSig = measure->timesig();
    const TimeSig* timeSigElement = findTimeSigElement(measure);
    const auto display = toMnxTimeSignatureDisplay(timeSigElement ? timeSigElement->timeSigType() : TimeSigType::NORMAL);

    // A change of glyph alone (2/2 to cut time, say) still needs to be emitted.
    if (prevTimeSig && timeSig.identical(*prevTimeSig) && display == prevDisplay) {
        return;
    }

    const auto unit = toMnxTimeSignatureUnit(timeSig.denominator());
    if (!unit) {
        LOGW() << "Skipping time signature with unsupported MNX time signature unit: " << timeSig.denominator();
        return;
    }

    auto mnxTimeSig = mnxMeasure.ensure_time(timeSig.numerator(), *unit);
    if (display) {
        mnxTimeSig.set_display(display.value());
    } else {
        mnxTimeSig.clear_display();
    }
    prevTimeSig = timeSig;
    prevDisplay = display;
}

//---------------------------------------------------------
//   assignKeySignature
//---------------------------------------------------------

static void assignKeySignature(mnx::global::Measure& mnxMeasure, const Score* score, const Measure* measure,
                               std::optional<int>& prevKeyFifths)
{
    if (score->staves().empty()) {
        return;
    }

    const Staff* staff = score->staff(0);
    const KeySigEvent keySigEvent = staff->keySigEvent(measure->tick());
    if (!keySigEvent.isValid()) {
        return;
    }

    const int keyFifths = static_cast<int>(keySigEvent.concertKey());
    if (keyFifths != prevKeyFifths) {
        mnxMeasure.ensure_key(keyFifths);
        prevKeyFifths = keyFifths;
    }

    prevKeyFifths = keyFifths;
}

//---------------------------------------------------------
//   assignBarline
//---------------------------------------------------------

static void assignBarline(mnx::global::Measure& mnxMeasure, const Measure* measure)
{
    if (!measure->endBarLineVisible()) {
        mnxMeasure.ensure_barline(mnx::BarlineType::NoBarline);
        return;
    }

    mnx::BarlineType barlineType = toMnxBarLineType(measure->endBarLineType());
    if (barlineType == mnx::BarlineType::Regular) {
        if (const BarLine* barline = measure->endBarLine()) {
            const int spanFrom = barline->spanFrom();
            const int spanTo = barline->spanTo();
            const bool isShort = (spanFrom == BARLINE_SPAN_SHORT1_FROM && spanTo == BARLINE_SPAN_SHORT1_TO)
                                 || (spanFrom == BARLINE_SPAN_SHORT2_FROM && spanTo == BARLINE_SPAN_SHORT2_TO);
            if (isShort) {
                barlineType = mnx::BarlineType::Short;
            } else if (const Staff* staff = barline->staff()) {
                const Fraction tick = barline->segment() ? barline->segment()->tick() : measure->tick();
                const int lines = staff->lines(tick - Fraction::eps()) - 1;
                const bool isOneLine = (lines <= 0);
                if (isOneLine) {
                    const int tickFrom = BARLINE_SPAN_TICK1_FROM + BARLINE_SPAN_1LINESTAFF_FROM;
                    const int tickTo = BARLINE_SPAN_1LINESTAFF_FROM + 1;
                    if (spanFrom == tickFrom && spanTo == tickTo) {
                        barlineType = mnx::BarlineType::Tick;
                    }
                } else {
                    const bool isTick = (spanFrom == BARLINE_SPAN_TICK1_FROM && spanTo == BARLINE_SPAN_TICK1_TO)
                                        || (spanFrom == BARLINE_SPAN_TICK2_FROM && spanTo == BARLINE_SPAN_TICK2_TO);
                    if (isTick) {
                        barlineType = mnx::BarlineType::Tick;
                    }
                }
            }
        }
    }
    const bool isLastMeasure = (measure->nextMeasure() == nullptr);

    if (isLastMeasure && barlineType != mnx::BarlineType::Final) {
        mnxMeasure.ensure_barline(barlineType);
    } else if (!isLastMeasure && barlineType != mnx::BarlineType::Regular) {
        mnxMeasure.ensure_barline(barlineType);
    }
}

//---------------------------------------------------------
//   assignRepeats
//---------------------------------------------------------

static void assignRepeats(mnx::global::Measure& mnxMeasure, const Measure* measure)
{
    if (measure->repeatStart()) {
        mnxMeasure.ensure_repeatStart();
    }

    if (measure->repeatEnd()) {
        auto repeatEnd = mnxMeasure.ensure_repeatEnd();
        const int repeatCount = measure->repeatCount();
        if (repeatCount > 0 && repeatCount != 2) {
            repeatEnd.set_times(repeatCount);
        }
    }
}

//---------------------------------------------------------
//   createTempo
//   emit a MNX tempo entry from a TempoText item
//---------------------------------------------------------

static void createTempo(mnx::global::Measure& mnxMeasure, const Measure* measure, const TempoText* tempo, const Fraction& relTick)
{
    IF_ASSERT_FAILED(measure && tempo) {
        return;
    }

    // The MNX tempo object is a playback directive, so a tempo text that MuseScore does not play
    // has nothing reliable to contribute (its stored tempo may be stale or unrelated to the text).
    /// @todo When MNX adds a display string to tempo, export non-playing tempo texts as display-only
    /// and include the original text for playing ones as well (as MusicXML does with words/sound).
    if (!tempo->playTempoText()) {
        return;
    }

    const auto location = toMnxFractionValue(relTick).reduced();

    // MuseScore stores the playback tempo in quarter notes per second, independent of how the text is
    // spelled. Use that rather than re-parsing the text, which fails for non-SMuFL tempo fonts.
    // "A tempo" and "Tempo primo" resolve to whatever the tempo map says is in effect at this tick.
    const BeatsPerSecond bps = (tempo->isATempo() || tempo->isTempoPrimo())
                               ? tempo->score()->tempo(tempo->tick())
                               : tempo->tempo();
    const double quarterBpm = bps.val * 60.0;
    if (quarterBpm <= 0.0) {
        LOGW() << "Skipping tempo with invalid beats-per-minute " << quarterBpm;
        return;
    }

    // Prefer the note value the text displays so the exported marking reads the same. If the text
    // cannot be parsed, use the beat of the current time signature (dotted for compound meters, unless
    // the tempo is slow enough that the subdivision is what gets beaten), and failing that a quarter
    // note, which is the unit the stored tempo is already in.
    const auto [dur, noteValue] = [&]() -> std::pair<TDuration, mnx::NoteValue::Required> {
        if (const TDuration textDur = tempo->duration(); textDur.isValid()) {
            if (const auto nv = toMnxNoteValue(textDur)) {
                return { textDur, *nv };
            }
        }
        if (const TimeSigFrac timeSig(measure->timesig()); timeSig.isValid() && timeSig.isNotZero()) {
            const int beatTicks = timeSig.isBeatedCompound(bps.val) ? timeSig.beatTicks() : timeSig.dUnitTicks();
            if (const TDuration beatDur(Fraction::fromTicks(beatTicks)); beatDur.isValid()) {
                if (const auto nv = toMnxNoteValue(beatDur)) {
                    return { beatDur, *nv };
                }
            }
        }
        return { TDuration(DurationType::V_QUARTER), mnx::NoteValue::make(mnx::NoteValueBase::Quarter) };
    }();
    // Smooth out floating-point noise from the unit conversions without imposing a fixed precision:
    // snap to the rounded value only when it is equal within MuseScore's relative epsilon.
    const double rawBpm = quarterBpm * (Fraction(1, 4) / dur.fraction()).toDouble();
    const double roundedBpm = muse::RealRound(rawBpm, 6);
    const double bpm = muse::RealIsEqual(rawBpm, roundedBpm) ? roundedBpm : rawBpm;

    auto mnxTempo = mnxMeasure.ensure_tempos().append(bpm, noteValue);
    if (relTick.isNotZero()) {
        mnxTempo.ensure_location(location);
    }
}

//---------------------------------------------------------
//   exportMeasureElements
//   export measure-level elements (jumps, markers, tempos)
//---------------------------------------------------------

static void exportMeasureElements(mnx::global::Measure& mnxMeasure, const Measure* measure)
{
    for (EngravingItem* item : measure->el()) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        const Fraction relTick = item->tick() - measure->tick();
        const auto location = toMnxFractionValue(relTick).reduced();
        const auto locationEnd = toMnxFractionValue(measure->ticks()).reduced(); // end of measure (relative)
        const auto locationOrEnd = relTick.isZero() ? locationEnd : location;

        switch (item->type()) {
        case ElementType::JUMP: {
            const Jump* jump = toJump(item);
            IF_ASSERT_FAILED(jump) {
                break;
            }
            if (const auto mnxJt = toMnxJumpType(jump->jumpType())) {
                /// @note Current jump types default to measure end when stored at tick 0.
                /// When MNX adds new types, revisit placement of new types.
                mnxMeasure.ensure_jump(*mnxJt, locationOrEnd);
            }
            break;
        }
        case ElementType::MARKER: {
            const Marker* marker = toMarker(item);
            IF_ASSERT_FAILED(marker) {
                break;
            }
            if (marker->markerType() == MarkerType::FINE) {
                mnxMeasure.ensure_fine(locationOrEnd);
            } else if (marker->isSegno()) {
                mnxMeasure.ensure_segno(location);
                /// @todo Export segno glyph when appropriate (defer pending MNX spec clarifications).
            }
            break;
        }
        default:
            break;
        }
    }

    for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
        for (const EngravingItem* item : segment->annotations()) {
            IF_ASSERT_FAILED(item) {
                continue;
            }

            const Fraction relTick = item->tick() - measure->tick();
            switch (item->type()) {
            case ElementType::TEMPO_TEXT:
                createTempo(mnxMeasure, measure, toTempoText(item), relTick);
                break;
            default:
                break;
            }
        }
    }

    if (const BarLine* barLine = measure->endBarLine()) {
        if (const Segment* seg = barLine->segment()) {
            for (const EngravingItem* item : seg->annotations()) {
                IF_ASSERT_FAILED(item) {
                    continue;
                }
                if (item->isFermata()) {
                    const Fermata* fermata = toFermata(item);
                    DO_ASSERT(fermata);
                    if (fermata) {
                        mnxMeasure.set_fermata(MnxExporter::mnxFermataFromFermata(fermata));
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   createGlobal
//---------------------------------------------------------

void MnxExporter::createGlobal()
{
    /// @note Lyrics metadata is exported at the end of `createParts`.
    /// @todo Export global sounds dictionary (mnx::global::Sound).

    if (!m_score) {
        return;
    }

    auto mnxMeasures = m_mnxDocument.global().measures();
    MeasureNumberState measureNumberState;
    std::optional<Fraction> prevTimeSig;
    std::optional<mnx::TimeSignatureDisplay> prevTimeSigDisplay;
    std::optional<int> prevKeyFifths;
    size_t measureIndex = 0;

    for (const Measure* measure = m_score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        auto mnxMeasure = mnxMeasures.append();
        mnxMeasure.set_id(getOrAssignEID(const_cast<Measure*>(measure)).toStdString());
        m_measToMnxMeas.emplace(measure, measureIndex);

        assignBarline(mnxMeasure, measure);
        assignRepeats(mnxMeasure, measure);
        assignTimeSignature(mnxMeasure, measure, prevTimeSig, prevTimeSigDisplay);
        assignKeySignature(mnxMeasure, m_score, measure, prevKeyFifths);
        exportMeasureElements(mnxMeasure, measure);

        const int displayNumber = measureNumberState.displayNumber(measure);
        if (displayNumber == 0) {
            /// @todo MNX does not support pickup measures; export as measure 0.
            mnxMeasure.set_number(displayNumber);
        } else if (displayNumber != static_cast<int>(measureIndex + 1)) {
            mnxMeasure.set_number(displayNumber);
        }

        ++measureIndex;
    }
}
} // namespace mu::iex::mnxio
