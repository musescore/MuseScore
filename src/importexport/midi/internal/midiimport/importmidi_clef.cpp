/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <set>

#include "importmidi_clef.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "importmidi_fraction.h"
#include "importmidi_meter.h"
#include "importmidi_operations.h"
#include "importmidi_tie.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::midi {
namespace MidiClef {
class AveragePitch
{
public:
    AveragePitch()
        : sumPitch_(0), count_(0) {}
    AveragePitch(int sumPitch, int count)
        : sumPitch_(sumPitch), count_(count) {}

    int pitch() const { return qRound(sumPitch_ * 1.0 / count_); }
    void addPitch(int pitch)
    {
        sumPitch_ += pitch;
        ++count_;
    }

    AveragePitch& operator+=(const AveragePitch& other)
    {
        sumPitch_ += other.sumPitch_;
        count_ += other.count_;
        return *this;
    }

private:
    int sumPitch_;
    int count_;
};

class MinMaxPitch
{
public:
    MinMaxPitch()
        : minPitch_(std::numeric_limits<int>::max()), maxPitch_(-1) {}
    MinMaxPitch(int minPitch, int maxPitch)
        : minPitch_(minPitch), maxPitch_(maxPitch) {}

    int minPitch() const { return minPitch_; }
    int maxPitch() const { return maxPitch_; }
    int empty() const { return minPitch_ == std::numeric_limits<int>::max() || maxPitch_ == -1; }
    void addPitch(int pitch)
    {
        if (pitch < minPitch_) {
            minPitch_ = pitch;
        }
        if (pitch > maxPitch_) {
            maxPitch_ = pitch;
        }
    }

private:
    int minPitch_;
    int maxPitch_;
};

int clefMidPitch()
{
    static const int midPitch = 60;
    return midPitch;
}

ClefType clefTypeFromAveragePitch(int averagePitch)
{
    return averagePitch < clefMidPitch() ? ClefType::F : ClefType::G;
}

//---------------------------------------------------------
//   createClef
//---------------------------------------------------------

static void createClef(ClefType clefType, Staff* staff, int tick, bool isSmall = false)
{
    if (tick == 0) {
        staff->setDefaultClefType(ClefTypeList(clefType, clefType));
    } else {
        Measure* m = staff->score()->tick2measure(Fraction::fromTicks(tick));
        Segment* seg = m->getSegment(SegmentType::Clef, Fraction::fromTicks(tick));
        Clef* clef = Factory::createClef(seg);
        clef->setClefType(clefType);
        const track_idx_t track = staff->idx() * VOICES;
        clef->setTrack(track);
        clef->setGenerated(false);
        clef->mutldata()->setMag(staff->staffMag(Fraction::fromTicks(tick)));
        clef->setSmall(isSmall);
        seg->add(clef);
    }
}

static AveragePitch findAverageSegPitch(const Segment* seg, track_idx_t strack)
{
    AveragePitch averagePitch;
    for (size_t voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = static_cast<ChordRest*>(seg->element(strack + voice));
        if (cr && cr->isChord()) {
            Chord* chord = toChord(cr);
            const auto& notes = chord->notes();
            for (const Note* note: notes) {
                averagePitch.addPitch(note->pitch());
            }
        }
    }
    return averagePitch;
}

static MinMaxPitch findMinMaxSegPitch(const Segment* seg, track_idx_t strack)
{
    MinMaxPitch minMaxPitch;
    for (size_t voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = static_cast<ChordRest*>(seg->element(strack + voice));
        if (cr && cr->isChord()) {
            Chord* chord = toChord(cr);
            const auto& notes = chord->notes();
            for (const Note* note: notes) {
                minMaxPitch.addPitch(note->pitch());
            }
        }
    }
    return minMaxPitch;
}

#ifdef QT_DEBUG

static bool doesClefBreakTie(const Staff* staff)
{
    const track_idx_t strack = staff->idx() * VOICES;

    for (size_t voice = 0; voice < VOICES; ++voice) {
        bool currentTie = false;
        for (Segment* seg = staff->score()->firstSegment(SegmentType::All); seg; seg = seg->next1()) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                if (MidiTie::isTiedBack(seg, strack, voice)) {
                    currentTie = false;
                }
                if (MidiTie::isTiedFor(seg, strack, voice)) {
                    currentTie = true;
                }
            } else if (seg->segmentType() == SegmentType::Clef && seg->element(strack)) {
                if (currentTie) {
                    LOGD() << "Clef breaks tie; measure number (from 1):"
                           << seg->measure()->no() + 1
                           << ", staff index (from 0):" << staff->idx();
                    return true;
                }
            }
        }
    }
    return false;
}

#endif

// clef index: 0 - treble, 1 - bass

int findPitchPenaltyForClef(int pitch, int clefIndex)
{
    const int farPitchPenalty = 10000;
    const int veryFarPitchPenalty = 20000;
    const int approxPitchPenalty = 1;
    const int dx = 5;

    const int midPitch = clefMidPitch();        // all notes equal or upper - better in G clef
    const int highPitch = midPitch + dx;        // almost all notes equal or upper - in G clef
    const int lowPitch = midPitch - dx;         // almost all notes lower - in F clef
    const int highestPitch = highPitch + dx;    // all notes equal or upper - in G clef
    const int lowestPitch = lowPitch - dx;      // all notes lower - in F clef

    switch (clefIndex) {
    case 0:
        if (pitch < lowestPitch) {
            return veryFarPitchPenalty;
        } else if (pitch < lowPitch) {
            return farPitchPenalty;
        } else if (pitch < midPitch) {
            return approxPitchPenalty;
        }
        break;
    case 1:
        if (pitch >= highestPitch) {
            return veryFarPitchPenalty;
        } else if (pitch >= highPitch) {
            return farPitchPenalty;
        } else if (pitch >= midPitch) {
            return approxPitchPenalty;
        }
        break;
    default:
        Q_ASSERT_X(false, "MidiClef::pitchPenalty", "Unknown clef type");
        break;
    }
    return 0;
}

static std::pair<ElementType, ReducedFraction> findChordRest(const Segment* seg, track_idx_t strack)
{
    ElementType elType = ElementType::INVALID;
    ReducedFraction newRestLen(0, 1);
    for (size_t voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = static_cast<ChordRest*>(seg->element(strack + voice));
        if (!cr) {
            continue;
        }
        if (cr->isChord()) {
            elType = ElementType::CHORD;
            break;
        } else if (cr->isRest()) {
            elType = ElementType::REST;
            newRestLen = qMax(newRestLen, ReducedFraction(cr->globalTicks()));
        }
    }
    return { elType, newRestLen };
}

int findClefChangePenalty(
    int pos,
    int clefIndex,
    const std::vector<std::vector<int> >& trebleBassPath,
    const Segment* segment,
    const Staff* staff)
{
    if (pos == 0) {
        return 0;
    }

    const int clefChangePenalty = 1000;
    const int prevClefChangePenalty = 10001;
    const int orphanChordPenalty = 2;
    const int notesBetweenClefs = 5;         // should be >= 2

    int j = pos;
    ReducedFraction totalRestLen(0, 1);
    int penalty = 0;
    const track_idx_t strack = staff->idx() * VOICES;
    const auto barFraction = ReducedFraction(
        staff->score()->sigmap()->timesig(segment->tick()).timesig());
    const ReducedFraction beatLen = Meter::beatLength(barFraction);

    // find backward penalty
    for (const Segment* segPrev = segment->prev1(SegmentType::ChordRest);;
         segPrev = segPrev->prev1(SegmentType::ChordRest)) {
        if (!segPrev) {
            penalty += clefChangePenalty;
            break;
        }
        const auto el = findChordRest(segPrev, strack);
        if (el.first == ElementType::CHORD) {
            --j;
            if (j == pos - notesBetweenClefs) {
                break;
            }
            if (j == 0 || trebleBassPath[clefIndex][j] != clefIndex) {
                if (j == pos - 1) {
                    penalty += prevClefChangePenalty;
                } else {
                    penalty += clefChangePenalty;
                }
                break;
            }
            totalRestLen = { 0, 1 };
        } else if (el.first == ElementType::REST) {
            totalRestLen += el.second;
            if (totalRestLen >= beatLen) {
                if (j != pos) {
                    penalty += orphanChordPenalty;
                }
                break;
            }
        }
    }
    // find forward penalty
    int chordCounter = 0;
    for (const Segment* seg = segment;; seg = seg->next1(SegmentType::ChordRest)) {
        if (!seg) {
            penalty += clefChangePenalty;
            break;
        }
        const auto el = findChordRest(seg, strack);
        if (el.first == ElementType::CHORD) {
            ++chordCounter;
            if (chordCounter == notesBetweenClefs) {
                break;
            }
            totalRestLen = { 0, 1 };
        } else if (el.first == ElementType::REST) {
            totalRestLen += el.second;
            if (totalRestLen >= beatLen) {
                penalty += orphanChordPenalty;
                break;
            }
        }
    }

    return penalty;
}

ClefType clefFromIndex(int index)
{
    return (index == 0) ? ClefType::G : ClefType::F;
}

void makeDynamicProgrammingStep(std::vector<std::vector<int> >& penalties,
                                std::vector<std::vector<int> >& optimalPaths,
                                int pos,
                                MidiTie::TieStateMachine::State tieState,
                                const MinMaxPitch& minMaxPitch,
                                const Segment* seg,
                                const Staff* staff)
{
    for (int clefIndex = 0; clefIndex != 2; ++clefIndex) {
        optimalPaths[clefIndex].resize(pos + 1);
    }

    for (int curClef = 0; curClef != 2; ++curClef) {
        const int significantPitch = (curClef == 0)
                                     ? minMaxPitch.minPitch() : minMaxPitch.maxPitch();
        const int pitchPenalty = findPitchPenaltyForClef(significantPitch, curClef);
        int minPenalty = std::numeric_limits<int>::max();
        int minIndex = 0;
        for (int prevClef = 0; prevClef != 2; ++prevClef) {
            int penalty = pitchPenalty;
            if (prevClef != curClef) {
                if (tieState == MidiTie::TieStateMachine::State::TIED_BACK
                    || tieState == MidiTie::TieStateMachine::State::TIED_BOTH) {
                    continue;             // there is a tie breakage that is incorrect
                }
                penalty += findClefChangePenalty(pos, prevClef, optimalPaths, seg, staff);
            }
            penalty += (pos > 0) ? penalties[prevClef][(pos + 1) % 2] : 0;
            if ((prevClef != curClef && penalty < minPenalty)
                || (prevClef == curClef && penalty <= minPenalty)) {
                minPenalty = penalty;
                minIndex = prevClef;
            }
        }

        penalties[curClef][pos % 2] = minPenalty;
        if (pos > 0) {
            optimalPaths[curClef][pos] = minIndex;
        }
    }
}

// clefs should be created from the beginning to the end of the score
// (otherwise clefs that are equal the default staff clef at the beginning
//  will be ignored),
// results of dynamic programming are collected from the end to the beginning
// so we need to store temporary results

bool createClefs(
    Staff* staff,
    const std::vector<std::vector<int> >& optimalPaths,
    int lastClef,
    const std::vector<Segment*>& segments)
{
    std::vector<std::pair<ClefType, int> > clefsAndTicks;

    int currentClef = lastClef;
    for (size_t i = optimalPaths[0].size() - 1; i; --i) {
        const int prevClef = optimalPaths[currentClef][i];
        if (prevClef != currentClef) {
            clefsAndTicks.push_back({ clefFromIndex(currentClef), segments[i]->tick().ticks() });
            currentClef = prevClef;
        }
    }
    if (clefsAndTicks.empty()) {
        return false;
    }

    createClef(clefFromIndex(currentClef), staff, 0);       // main staff clef
    for (size_t i = clefsAndTicks.size(); i; --i) {         // clef changes
        createClef(clefsAndTicks[i - 1].first, staff, clefsAndTicks[i - 1].second, true);
    }

    return true;
}

static void createMainClefFromAveragePitch(Staff* staff, track_idx_t strack)
{
    AveragePitch allAveragePitch;
    for (Segment* seg = staff->score()->firstSegment(SegmentType::ChordRest); seg;
         seg = seg->next1(SegmentType::ChordRest)) {
        allAveragePitch += findAverageSegPitch(seg, strack);
    }
    ClefType mainClef = clefTypeFromAveragePitch(allAveragePitch.pitch());
    createClef(mainClef, staff, 0);
}

bool hasGFclefs(const InstrumentTemplate* templ)
{
    if (!templ) {
        return false;
    }
    const size_t staveCount = templ->staffCount;
    bool hasG = false;
    bool hasF = false;
    for (staff_idx_t i = 0; i != staveCount; ++i) {
        switch (templ->clefTypes[i].concertClef) {
        case ClefType::G:
            hasG = true;
            break;
        case ClefType::F:
            hasF = true;
            break;
        default:
            break;
        }
    }
    return hasG && hasF;
}

void createClefs(Staff* staff, int indexOfOperation, bool isDrumTrack)
{
    const auto& opers = midiImportOperations.data()->trackOpers;
    const auto& trackInstrList = opers.msInstrList.value(indexOfOperation);
    const int msInstrIndex = opers.msInstrIndex.value(indexOfOperation);
    const bool hasInstrument = !trackInstrList.empty() && trackInstrList[msInstrIndex];

    if (isDrumTrack && !hasInstrument) {
        createClef(ClefType::PERC, staff, 0);
        return;
    }

    const track_idx_t strack = staff->idx() * VOICES;
    bool mainClefWasSet = false;
    const bool canChangeClef = !hasInstrument || hasGFclefs(trackInstrList[msInstrIndex]);

    if (opers.changeClef.value(indexOfOperation) && canChangeClef) {
        MidiTie::TieStateMachine tieTracker;

        // find optimal clef changes via dynamic programming
        std::vector<std::vector<int> > penalties(2);            // 0 - treble, 1 - bass
        // remember only 2 last positions to save memory
        for (size_t i = 0; i != penalties.size(); ++i) {
            penalties[i].resize(2);                             // 2 = current + prev
        }
        std::vector<std::vector<int> > optimalPaths(2);         // first col is unused
        std::vector<Segment*> segments;

        int pos = 0;
        for (Segment* seg = staff->score()->firstSegment(SegmentType::ChordRest); seg;
             seg = seg->next1(SegmentType::ChordRest)) {
            const auto minMaxPitch = findMinMaxSegPitch(seg, strack);
            if (minMaxPitch.empty()) {                          // no chords
                continue;
            }
            tieTracker.addSeg(seg, strack);
            segments.push_back(seg);

            makeDynamicProgrammingStep(penalties, optimalPaths, pos,
                                       tieTracker.state(), minMaxPitch, seg, staff);
            ++pos;
        }

        if (!optimalPaths[0].empty()) {
            const int lastClef = (penalties[1][(pos - 1) % 2] < penalties[0][(pos - 1) % 2])
                                 ? 1 : 0;
            // get the optimal clef changes found via dynamic programming
            if (createClefs(staff, optimalPaths, lastClef, segments)) {
                mainClefWasSet = true;
            }
        }
    }
    if (!mainClefWasSet && canChangeClef) {
        createMainClefFromAveragePitch(staff, strack);
    }
#ifdef QT_DEBUG
    Q_ASSERT_X(!doesClefBreakTie(staff), "MidiClef::createClefs", "Clef breaks the tie");
#endif
}
} // namespace MidiClef
} // namespace mu::iex::midi
