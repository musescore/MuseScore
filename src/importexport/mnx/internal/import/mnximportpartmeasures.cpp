/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include <algorithm>
#include <stack>
#include <vector>

#include "mnximporter.h"
#include "internal/shared/mnxtypesconv.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/note.h"
#include "engraving/dom/noteval.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/part.h"
#include "engraving/dom/partialtie.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/tiejumppointlist.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/volta.h"

#include "mnxdom.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
namespace {
struct LyricLineInterval {
    engraving::Fraction start;
    engraving::Fraction end;
};
} // namespace

//---------------------------------------------------------
//   intervalsOverlap
//   Return true if lyric intervals overlap.
//---------------------------------------------------------

static bool intervalsOverlap(const LyricLineInterval& a, const LyricLineInterval& b)
{
    return !(a.end < b.start || b.end < a.start);
}

//---------------------------------------------------------
//   tupletHasOnlySpacesAndGraces
//   Detect tuplets that contain only spaces and grace containers.
//---------------------------------------------------------

static bool tupletHasOnlySpacesAndGraces(const mnx::sequence::Tuplet& tuplet)
{
    const auto& content = tuplet.content();
    for (const auto& item : content) {
        if (item.type() == mnx::sequence::Space::ContentTypeValue) {
            continue;
        }
        if (item.type() == mnx::sequence::Grace::ContentTypeValue) {
            continue;
        }
        if (item.type() == mnx::sequence::Tuplet::ContentTypeValue) {
            const auto nested = item.get<mnx::sequence::Tuplet>();
            if (!tupletHasOnlySpacesAndGraces(nested)) {
                return false;
            }
            continue;
        }
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   createSlur
//   Create a MuseScore slur from an MNX slur attached to start chord/rest.
//---------------------------------------------------------

void MnxImporter::createSlur(const mnx::sequence::Slur& mnxSlur, engraving::ChordRest* startCR)
{
    ChordRest* targetCR = mnxEventIdToCR(mnxSlur.target());
    if (!targetCR) {
        LOGW() << "slur target was event with eventId " << mnxSlur.target() << " that was not mapped.";
        LOGW() << mnxSlur.dump(2);
        return;
    }
    Slur* slur = Factory::createSlur(m_score->dummy());
    slur->setScore(m_score);
    slur->setAnchor(Spanner::Anchor::CHORD);
    slur->setTrack(startCR->track());
    slur->setTrack2(targetCR->track());
    slur->setStartElement(startCR);
    slur->setEndElement(targetCR);
    slur->setTick(startCR->tick());
    slur->setTick2(targetCR->tick());
    m_score->addElement(slur);

    if (const auto lineType = mnxSlur.lineType()) {
        setAndStyleProperty(slur, Pid::SLUR_STYLE_TYPE, toMuseScoreSlurStyleType(lineType.value()));
    }
    if (const auto side = mnxSlur.side()) {
        DirectionV slurDir = side.value() == mnx::SlurTieSide::Up ? DirectionV::UP : DirectionV::DOWN;
        setAndStyleProperty(slur, Pid::SLUR_DIRECTION, slurDir);
    } else if (const auto sideEnd = mnxSlur.sideEnd()) {
        DirectionV slurDir = sideEnd.value() == mnx::SlurTieSide::Up ? DirectionV::UP : DirectionV::DOWN;
        setAndStyleProperty(slur, Pid::SLUR_DIRECTION, slurDir);
    }
    /// @todo implement side and sideEnd in opposite directions, if/when MuseScore supports it.
    /// @todo endNote and startNote are not supported by MuseScore (yet?)
}

//---------------------------------------------------------
//   buildLyricLineVerseMap
//   Map MNX lyric line ids to MuseScore verse numbers per staff.
//---------------------------------------------------------

void MnxImporter::buildLyricLineVerseMap()
{
    m_lyricLineToVerse.clear();

    const auto& lineOrder = mnxDocument().getEntityMap().getLyricLineOrder();
    std::unordered_map<std::string, size_t> lineOrderIndex;
    lineOrderIndex.reserve(lineOrder.size());
    for (size_t i = 0; i < lineOrder.size(); ++i) {
        lineOrderIndex.emplace(lineOrder[i], i);
    }

    const bool hasLineOrder = !lineOrderIndex.empty();
    for (auto& [staffIdx, lineUsage] : m_lyricLineUsage) {
        struct LineEntry {
            std::string id;
            LyricLineInterval interval;
        };
        std::vector<LineEntry> entries;
        entries.reserve(lineUsage.size());
        for (const auto& [lineId, usage] : lineUsage) {
            entries.push_back(LineEntry { lineId, { usage.first, usage.second } });
        }

        std::sort(entries.begin(), entries.end(),
                  [&](const LineEntry& a, const LineEntry& b) {
            if (hasLineOrder) {
                const auto aIt = lineOrderIndex.find(a.id);
                const auto bIt = lineOrderIndex.find(b.id);
                const bool aIn = aIt != lineOrderIndex.end();
                const bool bIn = bIt != lineOrderIndex.end();
                if (aIn && bIn && aIt->second != bIt->second) {
                    return aIt->second < bIt->second;
                }
                if (aIn != bIn) {
                    return aIn;
                }
            }
            if (a.interval.start < b.interval.start) {
                return true;
            }
            if (b.interval.start < a.interval.start) {
                return false;
            }
            return a.id < b.id;
        });

        std::vector<std::vector<LyricLineInterval> > verseIntervals;
        auto& lineToVerse = m_lyricLineToVerse[staffIdx];
        for (const auto& entry : entries) {
            size_t verseIndex = 0;
            for (; verseIndex < verseIntervals.size(); ++verseIndex) {
                bool overlaps = false;
                for (const auto& interval : verseIntervals[verseIndex]) {
                    if (intervalsOverlap(interval, entry.interval)) {
                        overlaps = true;
                        break;
                    }
                }
                if (!overlaps) {
                    break;
                }
            }
            if (verseIndex == verseIntervals.size()) {
                verseIntervals.emplace_back();
            }
            verseIntervals[verseIndex].push_back(entry.interval);
            lineToVerse.emplace(entry.id, static_cast<int>(verseIndex));
        }
    }
}

//---------------------------------------------------------
//   createLyrics
//   Create MuseScore lyrics for a chord/rest from an MNX event.
//---------------------------------------------------------

void MnxImporter::createLyrics(const mnx::sequence::Event& mnxEvent, engraving::ChordRest* cr)
{
    /// @todo import lyric line metadata (i.e., language code) somehow?
    if (const auto lyrics = mnxEvent.lyrics()) {
        if (const auto lines = lyrics->lines()) {
            const auto& mnxLineOrder = mnxDocument().getEntityMap().getLyricLineOrder();
            const auto staffIt = m_lyricLineToVerse.find(cr->staffIdx());
            if (!mnxLineOrder.empty()) {
                for (size_t lineIndex = 0; lineIndex < mnxLineOrder.size(); lineIndex++) {
                    const auto it = lines->find(mnxLineOrder[lineIndex]);
                    if (it == lines->end() || it->second.text().empty()) {
                        continue;
                    }
                    int verse = static_cast<int>(lineIndex);
                    if (staffIt != m_lyricLineToVerse.end()) {
                        const auto mapped = staffIt->second.find(mnxLineOrder[lineIndex]);
                        if (mapped != staffIt->second.end()) {
                            verse = mapped->second;
                        }
                    }
                    Lyrics* lyric = Factory::createLyrics(cr);
                    lyric->setTrack(cr->track());
                    lyric->setParent(cr);
                    lyric->setVerse(verse);
                    lyric->setXmlText(String::fromStdString(it->second.text()));
                    lyric->setSyllabic(toMuseScoreLyricsSyllabic(it->second.type()));
                    /// @todo styled text when supported by MNX.
                    /// @todo word extension span, if mnx ever provides it
                    cr->add(lyric);
                }
                return;
            }
            for (const auto& [lineId, line] : *lines) {
                if (line.text().empty()) {
                    continue;
                }
                Lyrics* lyric = Factory::createLyrics(cr);
                lyric->setTrack(cr->track());
                lyric->setParent(cr);
                int verse = 0;
                if (staffIt != m_lyricLineToVerse.end()) {
                    const auto mapped = staffIt->second.find(lineId);
                    if (mapped != staffIt->second.end()) {
                        verse = mapped->second;
                    }
                }
                lyric->setVerse(verse);
                lyric->setXmlText(String::fromStdString(line.text()));
                lyric->setSyllabic(toMuseScoreLyricsSyllabic(line.type()));
                /// @todo import word extension span, if mnx ever provides it
                cr->add(lyric);
            }
        }
    }
}

//---------------------------------------------------------
//   updateLyricLineUsageForEvent
//   Track lyric line intervals per staff for verse assignment.
//---------------------------------------------------------

void MnxImporter::updateLyricLineUsageForEvent(const mnx::sequence::Event& event, const mnx::Sequence& sequence,
                                               engraving::Measure* measure, engraving::track_idx_t curTrackIdx,
                                               const mnx::FractionValue& startTick)
{
    const auto lyrics = event.lyrics();
    if (!lyrics) {
        return;
    }
    const auto lines = lyrics->lines();
    if (!lines) {
        return;
    }
    const staff_idx_t baseStaffIdx = track2staff(curTrackIdx);
    const int eventStaff = event.staff_or(sequence.staff());
    int crossStaffMove = eventStaff - sequence.staff();
    // Cross-staff lyric events should reserve a verse on the staff where the event lands.
    staff_idx_t targetStaffIdx = static_cast<staff_idx_t>(int(baseStaffIdx) + crossStaffMove);
    if (!m_score->staff(targetStaffIdx)) {
        targetStaffIdx = baseStaffIdx;
    }
    const engraving::Fraction tick = measure->tick() + toMuseScoreFraction(startTick);
    auto& lineUsage = m_lyricLineUsage[targetStaffIdx];
    for (const auto& [lineId, line] : *lines) {
        if (line.text().empty()) {
            continue;
        }
        auto it = lineUsage.find(lineId);
        if (it == lineUsage.end()) {
            lineUsage.emplace(lineId, std::make_pair(tick, tick));
        } else {
            if (tick < it->second.first) {
                it->second.first = tick;
            }
            if (it->second.second < tick) {
                it->second.second = tick;
            }
        }
    }
}

//---------------------------------------------------------
//   createTies
//   Create MuseScore ties (including laissez vibrer and jump ties) from MNX tie data.
//---------------------------------------------------------

void MnxImporter::createTies(const mnx::Array<mnx::sequence::Tie>& ties, engraving::Note* startNote)
{
    bool createdNonJumpTie = false;
    for (const mnx::sequence::Tie& mnxTie : ties) {
        if (mnxTie.targetType() == mnx::TieTargetType::CrossJump) {
            continue;
        }

        const bool isLv = mnxTie.lv() || !mnxTie.target();
        Note* targetNote = nullptr;
        if (!isLv) {
            const auto target = mnxTie.target();
            targetNote = target ? mnxNoteIdToNote(target.value()) : nullptr;
            if (!targetNote) {
                if (target) {
                    LOGW() << "Skipping tie to note with noteId " << target.value() << " that was not mapped.";
                    LOGW() << mnxTie.dump(2);
                }
                continue;
            }
        }

        Tie* tie = isLv ? Factory::createLaissezVib(startNote) : Factory::createTie(startNote);
        tie->setStartNote(startNote);
        tie->setTick(startNote->tick());
        tie->setTrack(startNote->track());
        tie->setParent(startNote);
        startNote->setTieFor(tie);
        DirectionV tieDir = DirectionV::AUTO;
        if (const auto side = mnxTie.side()) {
            tieDir = side.value() == mnx::SlurTieSide::Up ? DirectionV::UP : DirectionV::DOWN;
            setAndStyleProperty(tie, Pid::SLUR_DIRECTION, tieDir);
        }
        if (!isLv) {
            tie->setEndNote(targetNote);
            tie->setTick2(targetNote->tick());
            tie->setTrack2(targetNote->track());
            targetNote->setTieBack(tie);
        }
        createdNonJumpTie = true;
        break;
    }

    Tie* startTie = startNote->tieFor();
    if (createdNonJumpTie && startTie && startTie->isLaissezVib()) {
        return;
    }

    struct JumpTieTarget {
        mnx::sequence::Tie tie;
        Note* targetNote = nullptr;
    };
    std::vector<JumpTieTarget> jumpTargets;
    for (const mnx::sequence::Tie& mnxTie : ties) {
        if (mnxTie.targetType() != mnx::TieTargetType::CrossJump) {
            continue;
        }

        const auto target = mnxTie.target();
        Note* targetNote = target ? mnxNoteIdToNote(target.value()) : nullptr;
        if (!targetNote) {
            if (target) {
                LOGW() << "tie target was note with noteId " << target.value() << " that was not mapped.";
                LOGW() << mnxTie.dump(2);
            }
            continue;
        }
        jumpTargets.push_back(JumpTieTarget { mnxTie, targetNote });
    }

    // sort may not be strictly necessary
    std::sort(jumpTargets.begin(), jumpTargets.end(),
              [](const JumpTieTarget& a, const JumpTieTarget& b) {
        if (a.targetNote->tick() != b.targetNote->tick()) {
            return a.targetNote->tick() < b.targetNote->tick();
        }
        return a.targetNote->track() < b.targetNote->track();
    });

    for (const JumpTieTarget& jumpTarget : jumpTargets) {
        const mnx::sequence::Tie& mnxTie = jumpTarget.tie;
        Note* targetNote = jumpTarget.targetNote;

        DirectionV tieDir = DirectionV::AUTO;
        if (const auto side = mnxTie.side()) {
            tieDir = side.value() == mnx::SlurTieSide::Up ? DirectionV::UP : DirectionV::DOWN;
        }
        if (!startTie || startTie->isPartialTie()) {
            PartialTie* tie = Factory::createPartialTie(startNote);
            tie->setStartNote(startNote);
            tie->setTick(startNote->tick());
            tie->setTrack(startNote->track());
            tie->setParent(startNote);
            startNote->setTieFor(tie);
            startTie = tie;
            setAndStyleProperty(tie, Pid::SLUR_DIRECTION, tieDir);
        }

        TieJumpPointList* jumpPoints = startTie->tieJumpPoints();
        IF_ASSERT_FAILED(jumpPoints) {
            continue;
        }
        const int jumpPointIdx = static_cast<int>(jumpPoints->size());
        TieJumpPoint* jumpPoint = new TieJumpPoint(targetNote, true, jumpPointIdx, false);
        jumpPoints->add(jumpPoint);
        jumpPoints->undoAddTieToScore(jumpPoint);
        if (Tie* endTie = jumpPoint->endTie()) {
            setAndStyleProperty(endTie, Pid::SLUR_DIRECTION, tieDir);
        }
    }
}

//---------------------------------------------------------
//   createAccidentals
//   Apply MNX accidental display to a MuseScore note.
//---------------------------------------------------------

void MnxImporter::createAccidentals(const mnx::sequence::Note& mnxNote, Note* note, Measure* measure)
{
    const auto accidentalDisplay = mnxNote.accidentalDisplay();
    bool forceAccidental = accidentalDisplay && accidentalDisplay->force();
    bool showAccidental = accidentalDisplay ? accidentalDisplay->show() : false;
    bool hasEnclosure = false;
    mnx::AccidentalEnclosureSymbol enclosureSymbol = mnx::AccidentalEnclosureSymbol::Parentheses;
    if (accidentalDisplay) {
        if (const auto enclosure = accidentalDisplay->enclosure()) {
            hasEnclosure = true;
            enclosureSymbol = enclosure->symbol();
        }
    }

    if (!forceAccidental) {
        if (!m_useAccidentalDisplay) {
            return;
        }
        IF_ASSERT_FAILED(measure) {
            return;
        }
        AccidentalVal accVal = tpc2alter(note->tpc());
        AccidentalVal currentVal = measure->findAccidental(note);
        // autoShow mirrors the default accidental state at this note before forcing any override.
        bool autoShow = accVal != currentVal;
        if (autoShow == showAccidental) {
            return;
        }
    }

    AccidentalVal accVal = tpc2alter(note->tpc());
    AccidentalType accType = Accidental::value2subtype(accVal);
    Accidental* accidental = Factory::createAccidental(note);
    accidental->setAccidentalType(accType);
    accidental->setRole(AccidentalRole::USER);
    accidental->setVisible(showAccidental);
    if (showAccidental && hasEnclosure) {
        accidental->setBracket(enclosureSymbol == mnx::AccidentalEnclosureSymbol::Parentheses
                               ? AccidentalBracket::PARENTHESIS
                               : AccidentalBracket::BRACKET);
    }
    note->add(accidental);
}

//---------------------------------------------------------
//   createRestPosition
//   Position a rest vertically when MNX supplies staffPosition.
//---------------------------------------------------------

void MnxImporter::createRestPosition(const mnx::sequence::Rest& mnxRest, Rest* rest)
{
    if (const auto staffPosition = mnxRest.staffPosition()) {
        /// @todo Revisit rest positioning if MuseScore exposes a straightforward staff-line override.
        rest->setAlignWithOtherRests(false);
        rest->setMinDistance(Spatium(-999.0));
        const double lineDist = rest->staff()->lineDistance(rest->tick());
        // MNX staffPosition is in half-spaces relative to the middle line.
        rest->ryoffset() = -staffPosition.value() * lineDist * rest->spatium() / 2.0;
    }
}

//---------------------------------------------------------
//   emitGapRest
//   Emit a spacer (gap) rest for MNX space content.
//---------------------------------------------------------

Rest* MnxImporter::emitGapRest(Measure* measure, track_idx_t curTrackIdx,
                               const mnx::FractionValue& startTick, const mnx::FractionValue& duration,
                               Tuplet* tupletToAdd)
{
    Segment* segment = measure->getSegmentR(SegmentType::ChordRest, toMuseScoreFraction(startTick));
    TDuration d(toMuseScoreFraction(duration));
    if (!d.isValid()) {
        return nullptr;
    }
    Rest* rest = Factory::createRest(segment, d);
    rest->setDurationType(d);
    rest->setGap(true);
    rest->setTrack(curTrackIdx);
    rest->setTicks(rest->actualDurationType().fraction());
    segment->add(rest);
    if (tupletToAdd) {
        tupletToAdd->add(rest);
    }
    return rest;
}

//---------------------------------------------------------
//   createNote
//   Create a MuseScore note from an MNX note, applying transposition.
//---------------------------------------------------------

Note* MnxImporter::createNote(const mnx::sequence::Note& mnxNote, Chord* chord, Staff* baseStaff,
                              const Fraction& tick, int ottavaDisplacement, track_idx_t curTrackIdx)
{
    Note* note = Factory::createNote(chord);
    note->setParent(chord);
    note->setTrack(curTrackIdx);
    auto pitch = mnxNote.pitch();
    NoteVal nval = toMuseScoreNoteVal(pitch, baseStaff->concertKey(tick), ottavaDisplacement);
    // calcTransposed accounts for MNX transposeWritten.
    NoteVal nvalTransposed = toMuseScoreNoteVal(pitch.calcTransposed(), baseStaff->key(tick), ottavaDisplacement);
    nval.tpc2 = nvalTransposed.tpc2;
    note->setNval(nval);
    chord->add(note);
    m_mnxNoteToNote.emplace(mnxNote.pointer().to_string(), note);
    return note;
}

//---------------------------------------------------------
//   createTuplet
//   Create a MuseScore tuplet from an MNX tuplet container.
//---------------------------------------------------------

Tuplet* MnxImporter::createTuplet(const mnx::sequence::Tuplet& mnxTuplet, Measure* measure, track_idx_t curTrackIdx)
{
    TDuration baseLen = toMuseScoreDuration(mnxTuplet.outer().duration());
    mnx::FractionValue ratioDivisor = mnxTuplet.outer() / mnxTuplet.inner().duration();
    if (!baseLen.isValid() || ratioDivisor.remainder() != 0) {
        LOGE() << "Unable to import tuplet at " << mnxTuplet.pointer().to_string();
        LOGE() << mnxTuplet.dump(2);
        return nullptr;
    }
    Fraction tupletRatio = Fraction(mnxTuplet.inner().multiple(), ratioDivisor.quotient());

    Tuplet* t = Factory::createTuplet(measure);
    t->setTrack(curTrackIdx);
    t->setParent(measure);
    t->setRatio(tupletRatio);
    t->setBaseLen(baseLen);
    Fraction f = baseLen.fraction() * tupletRatio.denominator();
    t->setTicks(f.reduced());
    // options
    t->setNumberType(toMuseScoreTupletNumberType(mnxTuplet.showNumber()));
    t->setBracketType(toMuseScoreTupletBracketType(mnxTuplet.bracket()));
    return t;
}

//---------------------------------------------------------
//   createTremolo
//   Create a two-note tremolo from an MNX MultiNoteTremolo.
//---------------------------------------------------------

void MnxImporter::createTremolo(const mnx::sequence::MultiNoteTremolo& mnxTremolo,
                                Measure* measure, track_idx_t curTrackIdx,
                                const mnx::FractionValue& startTick, const mnx::FractionValue& endTick)
{
    const auto startTick2 = startTick + (endTick - startTick) / 2;
    engraving::Chord* c1 = measure->findChord(measure->tick() + toMuseScoreFraction(startTick), curTrackIdx);
    engraving::Chord* c2 = measure->findChord(measure->tick() + toMuseScoreFraction(startTick2), curTrackIdx);
    IF_ASSERT_FAILED(c1 && c2 && c1->ticks() == c2->ticks()) {
        LOGE() << "Unable to import tremolo at " << mnxTremolo.pointer().to_string();
        LOGE() << mnxTremolo.dump(2);
        return;
    }
    int tremoloBeamsNum = int(TremoloType::C8) - 1 + mnxTremolo.marks();
    tremoloBeamsNum = std::clamp(tremoloBeamsNum, int(TremoloType::C8), int(TremoloType::C64));
    if (tremoloBeamsNum <= c1->durationType().hooks()) {
        return; // no tremolo is possible
    }

    Fraction d = c1->ticks() / 2;
    c1->setDurationType(d.reduced());
    c1->setTicks(c1->actualDurationType().fraction());
    c2->setDurationType(c1->durationType());
    c2->setTicks(c1->ticks());

    if (c1->durationType().hooks() > 0) {
        // mnx does not include these tremolo events in beams, so do it here.
        c1->setBeamMode(BeamMode::BEGIN);
        c2->setBeamMode(BeamMode::END);
    }

    TremoloTwoChord* tremolo = Factory::createTremoloTwoChord(c1);
    tremolo->setTremoloType(TremoloType(tremoloBeamsNum));
    tremolo->setTrack(curTrackIdx);
    tremolo->setVisible(c1->notes().front()->visible());
    tremolo->setParent(c1);
    tremolo->setChords(c1, c2);
    c1->setTremoloTwoChord(tremolo);
}

//---------------------------------------------------------
//   importEvent
//   Return the created ChordRest when importing an MNX event (nullptr if skipped).
//---------------------------------------------------------

ChordRest* MnxImporter::importEvent(const mnx::sequence::Event& event,
                                    track_idx_t curTrackIdx, Measure* measure, const mnx::FractionValue& startTick,
                                    const std::stack<Tuplet*>& activeTuplets, TremoloTwoChord* activeTremolo)
{
    auto d = [&]() -> TDuration {
        if (const auto& duration = event.duration()) {
            return toMuseScoreDuration(duration.value());
        } else if (event.measure() && event.rest()) {
            return TDuration(DurationType::V_MEASURE);
        }
        return {};
    }();
    if (!d.isValid()) {
        LOGW() << "Given ChordRest duration not supported in MuseScore";
        return nullptr;
    }

    const engraving::Fraction eventTick = toMuseScoreFraction(startTick);
    Segment* segment = measure->getSegmentR(SegmentType::ChordRest, eventTick);
    mnx::Sequence sequence = event.getSequence();

    ChordRest* cr = nullptr;
    const int eventStaff = event.staff_or(sequence.staff());
    int crossStaffMove = eventStaff - sequence.staff();
    staff_idx_t staffIdx = track2staff(curTrackIdx);
    Staff* baseStaff = m_score->staff(staffIdx);
    Staff* targetStaff = baseStaff;
    if (crossStaffMove != 0) {
        const staff_idx_t targetStaffIdxCandidate = static_cast<staff_idx_t>(int(staffIdx) + crossStaffMove);
        Staff* candidateStaff = m_score->staff(targetStaffIdxCandidate);
        const bool canUseCandidate = candidateStaff && candidateStaff->visible()
                                     && candidateStaff->isLinked() == baseStaff->isLinked()
                                     && staff2track(staffIdx) >= baseStaff->part()->startTrack()
                                     && staff2track(targetStaffIdxCandidate) < baseStaff->part()->endTrack()
                                     && candidateStaff->staffType(eventTick)->group() == baseStaff->staffType(eventTick)->group();
        if (canUseCandidate) {
            targetStaff = candidateStaff;
        } else {
            crossStaffMove = 0;
        }
    }
    IF_ASSERT_FAILED(baseStaff && targetStaff) {
        LOGE() << "Event " << event.pointer().to_string() << " has invalid staff " << eventStaff << ".";
        return nullptr;
    }

    if (const auto& mnxRest = event.rest()) {
        Rest* rest = Factory::createRest(segment, d);
        cr = toChordRest(rest);
    } else {
        const auto& notes = event.notes();
        const auto& kitNotes = event.kitNotes();
        const int ottavaDisplacement = mnxDocument().getEntityMap().getOttavaShift(event);
        if ((notes && !notes->empty()) || (kitNotes && !kitNotes->empty())) {
            engraving::Chord* chord = Factory::createChord(segment);
            if (notes && !notes->empty()) {
                for (size_t i = 0; i < event.notes()->size(); i++) {
                    createNote(notes->at(i), chord, baseStaff, segment->tick(), ottavaDisplacement, curTrackIdx);
                }
            }
            if (kitNotes && !kitNotes->empty()) {
                const auto partIt = m_StaffToMnxPart.find(staffIdx);
                const Drumset* drumset = targetStaff->part()->instrument()->drumset();
                for (const auto& kitNote : kitNotes.value()) {
                    int midiPitch = -1;
                    if (partIt != m_StaffToMnxPart.end()) {
                        auto kitIt = m_mnxKitComponentToMidi.find({ partIt->second, kitNote.kitComponent() });
                        if (kitIt != m_mnxKitComponentToMidi.end()) {
                            midiPitch = kitIt->second;
                        }
                    }
                    if (!pitchIsValid(midiPitch)) {
                        LOGW() << "Kit note has unknown kit component \"" << kitNote.kitComponent()
                               << "\" at " << kitNote.pointer().to_string();
                        continue;
                    }
                    NoteVal nval;
                    nval.pitch = midiPitch;
                    if (drumset && drumset->isValid(midiPitch)) {
                        nval.headGroup = drumset->noteHead(midiPitch);
                    }
                    engraving::Note* note = Factory::createNote(chord);
                    note->setParent(chord);
                    note->setTrack(curTrackIdx);
                    note->setNval(nval);
                    chord->add(note);
                    m_mnxNoteToNote.emplace(kitNote.pointer().to_string(), note);
                }
            }
            if (chord->notes().empty()) {
                LOGW() << "Event " << event.pointer().to_string() << " has no valid notes.";
                delete chord;
                return nullptr;
            }
            if (const auto stemDir = event.stemDirection()) {
                chord->setStemDirection(stemDir.value() == mnx::StemDirection::Up ? DirectionV::UP : DirectionV::DOWN);
            }
            if (m_useBeams && d.hooks() > 0 && !mnxDocument().getEntityMap().tryGetBeam(event)) {
                chord->setBeamMode(BeamMode::NONE);
            }
            cr = toChordRest(chord);
        } else {
            LOGW() << "Event " << event.pointer().to_string() << " is neither rest nor chord.";
            return nullptr;
        }
    }
    cr->setDurationType(d);
    cr->setStaffMove(crossStaffMove);
    cr->setTrack(curTrackIdx);
    if (cr->durationType().isMeasure()) {
        cr->setTicks(measure->stretchedLen(baseStaff)); // baseStaff because that's the staff the cr 'belongs to'
    } else {
        cr->setTicks(cr->actualDurationType().fraction());
    }
    importMarkings(event, cr);
    if (!event.isGrace()) {
        segment->add(cr);
        if (!activeTuplets.empty()) {
            DO_ASSERT(activeTuplets.top());
            if (activeTuplets.top()) {
                activeTuplets.top()->add(cr);
            }
        }
        if (activeTremolo) {
            activeTremolo->add(cr);
        }
    }
    m_mnxEventToCR.emplace(event.pointer().to_string(), cr);
    return cr;
}

//---------------------------------------------------------
//   importNonGraceEvents
//   Import non-grace events and return true if any ChordRest was created.
//---------------------------------------------------------

bool MnxImporter::importNonGraceEvents(const mnx::Sequence& sequence, Measure* measure,
                                       track_idx_t curTrackIdx, GraceNeighborsMap& graceNeighbors)
{
    bool insertedCR = false;
    std::stack<Tuplet*> activeTuplets;
    TremoloTwoChord* activeTremolo = nullptr;

    ChordRest* lastCR = nullptr;
    std::vector<std::string> pendingNext;

    mnx::util::SequenceWalkHooks hooks;
    hooks.onItem = [&](const mnx::ContentObject& item, mnx::util::SequenceWalkContext& ctx) {
        if (item.type() == mnx::sequence::Grace::ContentTypeValue) {
            /// @todo refactor this if MuseScore allows grace notes to be normal.
            const auto grace = item.get<mnx::sequence::Grace>();
            const std::string key = grace.pointer().to_string();
            graceNeighbors[key] = { lastCR, nullptr }; // store prev neighbor
            pendingNext.push_back(key);
            return mnx::util::SequenceWalkControl::SkipChildren;
        } else if (item.type() == mnx::sequence::Tuplet::ContentTypeValue) {
            const auto mnxTuplet = item.get<mnx::sequence::Tuplet>();
            if (tupletHasOnlySpacesAndGraces(mnxTuplet)) {
                // MuseScore does not like Tuplets that contain only gap Rests,
                // so replace the entire thing with a single gap rest.
                TDuration baseLen = toMuseScoreDuration(mnxTuplet.outer().duration());
                const Fraction total = baseLen.fraction() * mnxTuplet.outer().multiple();
                DO_ASSERT(activeTuplets.empty());
                emitGapRest(measure, curTrackIdx, ctx.elapsedTime, toMnxFractionValue(total.reduced()), nullptr);
                activeTuplets.push(nullptr);
                return mnx::util::SequenceWalkControl::SkipChildren;
            }
            if (Tuplet* t = createTuplet(mnxTuplet, measure, curTrackIdx)) {
                if (!activeTuplets.empty()) {
                    DO_ASSERT(activeTuplets.top());
                    if (activeTuplets.top()) {
                        activeTuplets.top()->add(t); // reparent tuplet
                    }
                }
                activeTuplets.push(t);
            }
        } else if (item.type() == mnx::sequence::MultiNoteTremolo::ContentTypeValue) {
            const auto mnxTremolo = item.get<mnx::sequence::MultiNoteTremolo>();
            auto content = mnxTremolo.content();
            if (content.size() != 2) {
                LOGE() << "Tremolo at " << mnxTremolo.pointer().to_string() << " has " << content.size()
                       << " events and cannot be imported.";
                LOGE() << mnxTremolo.dump(2);
                return mnx::util::SequenceWalkControl::SkipChildren;
            }
            using MnxEv = mnx::sequence::Event;
            if (content[0].type() != MnxEv::ContentTypeValue || content[1].type() != MnxEv::ContentTypeValue) {
                LOGE() << "Tremolo at " << mnxTremolo.pointer().to_string() << " contains other content than events";
                LOGE() << mnxTremolo.dump(2);
                return mnx::util::SequenceWalkControl::SkipChildren;
            }
        } else if (item.type() == mnx::sequence::Space::ContentTypeValue) {
            // Note that if we are inside a tuplet (ctx.timeRatio != 1), we must emit a gap rest here.
            // For now, though, we emit a gap rest for *any* spacer, since that seems to be the intent of them.
            const auto mnxSpace = item.get<mnx::sequence::Space>();
            emitGapRest(measure, curTrackIdx, ctx.elapsedTime, mnxSpace.duration(),
                        activeTuplets.empty() ? nullptr : activeTuplets.top());
        }
        return mnx::util::SequenceWalkControl::Continue;
    };
    hooks.onEvent = [&](const mnx::sequence::Event& event,
                        const mnx::FractionValue& startTick,
                        const mnx::FractionValue&, [[maybe_unused]] mnx::util::SequenceWalkContext& ctx) {
        IF_ASSERT_FAILED(!ctx.inGrace) {
            LOGE() << "Encountered grace when processing non-grace.";
            return true;
        }
        updateLyricLineUsageForEvent(event, sequence, measure, curTrackIdx, startTick);
        if (ChordRest* cr = importEvent(event, curTrackIdx, measure, startTick, activeTuplets, activeTremolo)) {
            lastCR = cr;
            insertedCR = true;
            for (const auto& key : pendingNext) {
                auto it = graceNeighbors.find(key);
                if (it != graceNeighbors.end()) {
                    it->second.second = cr; // store next neighbor
                }
            }
            pendingNext.clear();
        }
        return true;
    };
    hooks.onAfterItem = [&](const mnx::ContentObject& item, mnx::util::SequenceWalkContext& ctx) {
        if (item.type() == mnx::sequence::Tuplet::ContentTypeValue) {
            activeTuplets.pop();
        } else if (item.type() == mnx::sequence::MultiNoteTremolo::ContentTypeValue) {
            const auto mnxTremolo = item.get<mnx::sequence::MultiNoteTremolo>();
            const auto startTime = ctx.elapsedTime - (mnxTremolo.outer() * ctx.timeRatio);
            createTremolo(mnxTremolo, measure, curTrackIdx, startTime, ctx.elapsedTime);
        }
    };

    mnx::util::walkSequenceContent(sequence, hooks);

    return insertedCR;
}

//---------------------------------------------------------
//   importGraceEvents
//   Import grace-note events using neighbor context.
//---------------------------------------------------------

void MnxImporter::importGraceEvents(const mnx::Sequence& sequence, Measure* measure,
                                    track_idx_t curTrackIdx, const GraceNeighborsMap& graceNeighbors)
{
    mnx::util::SequenceWalkHooks hooks;
    hooks.onEvent = [&](const mnx::sequence::Event& event,
                        const mnx::FractionValue& startTick,
                        const mnx::FractionValue&, mnx::util::SequenceWalkContext& ctx) {
        if (ctx.inGrace) {
            if (event.rest()) {
                LOGW() << "encountered unsupported grace note rest at " << event.pointer().to_string();
                return true;
            }
            auto grace = event.container<mnx::sequence::Grace>();
            auto [leftNeighbor, rightNeighbor] = muse::value(graceNeighbors, grace.pointer().to_string());
            const bool useRight = rightNeighbor && rightNeighbor->isChord();
            const bool useLeft = !useRight && leftNeighbor && leftNeighbor->isChord();
            if (useRight || useLeft) {
                if (ChordRest* cr = importEvent(event, curTrackIdx, measure, startTick, {}, nullptr)) {
                    engraving::Chord* gc = toChord(cr);
                    TDuration d = gc->durationType();
                    if (useRight && grace.slash() && grace.content().size() == 1) {
                        gc->setNoteType(engraving::NoteType::ACCIACCATURA);
                    } else {
                        gc->setNoteType(duraTypeToGraceNoteType(d.type(), useLeft));
                        gc->setShowStemSlash(grace.slash());
                    }
                    if (useRight) {
                        Chord* graceParent = toChord(rightNeighbor);
                        gc->setGraceIndex(graceParent->graceNotesBefore().size());
                        graceParent->add(gc);
                    } else if (useLeft) {
                        Chord* graceParent = toChord(leftNeighbor);
                        gc->setGraceIndex(0);
                        graceParent->add(gc);
                    }
                }
            }
        }
        return true;
    };

    mnx::util::walkSequenceContent(sequence, hooks);
}

//---------------------------------------------------------
//   importSequences
//   Import all sequences for a part measure into a MuseScore measure.
//---------------------------------------------------------

void MnxImporter::importSequences(const mnx::Part& mnxPart, const mnx::part::Measure& partMeasure,
                                  Measure* measure)
{
    std::vector<std::vector<track_idx_t> > staffVoiceMaps(mnxPart.staves());

    // pass1: import non-grace-note events to ChordRest
    for (const auto& sequence : partMeasure.sequences()) {
        if (sequence.staff() > mnxPart.staves()) {
            LOGE() << "Sequence " << sequence.pointer().to_string()
                   << " specifies non-existent staff " << sequence.staff()
                   << " for MNX part at " << mnxPart.pointer().to_string() << ".";
            continue;
        }
        const staff_idx_t curStaffIdx = muse::value(m_mnxPartStaffToStaff,
                                                    std::make_pair(mnxPart.calcArrayIndex(), sequence.staff()),
                                                    muse::nidx);
        IF_ASSERT_FAILED(curStaffIdx != muse::nidx) {
            LOGE() << "Sequence " << sequence.pointer().to_string()
                   << " specifies unmapped staff " << sequence.staff()
                   << " for MNX part at " << mnxPart.pointer().to_string() << ".";
            return;
        }
        auto& staffVoiceMap = staffVoiceMaps[static_cast<size_t>(sequence.staff() - 1)];
        const track_idx_t voiceId = staffVoiceMap.size();
        if (voiceId >= VOICES) {
            LOGW() << "Part measure " << partMeasure.pointer().to_string()
                   << " contains too many voices for staff " << sequence.staff() << ". This sequence is skipped.";
        }
        const track_idx_t curTrackIdx = staff2track(curStaffIdx, voiceId);
        GraceNeighborsMap graceNeighbors;
        if (importNonGraceEvents(sequence, measure, curTrackIdx, graceNeighbors)) {
            importGraceEvents(sequence, measure, curTrackIdx, graceNeighbors); // if MuseScore refactors graces, maybe we don't need this.
            staffVoiceMap.push_back(voiceId);
        }
    }

    // fill in measures as needed with rests.
    for (int staffNum = 1; staffNum <= mnxPart.staves(); staffNum++) {
        staff_idx_t staffIdx = mnxPartStaffToStaffIdx(mnxPart, staffNum);
        measure->checkMeasure(staffIdx);
    }
}

//---------------------------------------------------------
//   createDynamics
//   Create MuseScore dynamics from MNX part-measure dynamics.
//---------------------------------------------------------

void MnxImporter::createDynamics(const mnx::part::Measure& mnxMeasure, engraving::Measure* measure)
{
    const auto part = mnxMeasure.getEnclosingElement<mnx::Part>();
    if (const auto mnxDynamics = mnxMeasure.dynamics()) {
        for (const auto& mnxDynamic : mnxDynamics.value()) {
            /// @todo Process all dynamics, including those without glyphs, once the meaning of value()
            /// has been clarified and once mnx has text formatting, which seems to be imminent.
            if (!mnxDynamic.glyph()) {
                continue;
            }
            /// @todo Honor mnx requirement that dynamics apply to all staves when staff() member
            /// is missing (after clarification).
            staff_idx_t staffIdx = muse::value(m_mnxPartStaffToStaff,
                                               std::make_pair(part->calcArrayIndex(), mnxDynamic.staff_or(1)),
                                               muse::nidx);
            IF_ASSERT_FAILED(staffIdx != muse::nidx) {
                LOGE() << "staff idx not found for part " << part->pointer().to_string();
                continue;
            }
            track_idx_t curTrackIdx = staff2track(staffIdx);

            Fraction rTick = toMuseScoreFraction(mnxDynamic.position().fraction());
            Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick() + rTick);
            Dynamic* dyn = Factory::createDynamic(s);
            dyn->setParent(s);
            dyn->setTrack(curTrackIdx);
            /// @todo: smarter approach to creating xmlText.
            String xmlText = u"<sym>" + String::fromStdString(mnxDynamic.glyph().value()) + u"</sym>";
            dyn->setXmlText(xmlText);
            dyn->setDynamicType(toMuseScoreDynamicType(xmlText));
            /// @todo: voice assignment based on voice()
            dyn->setVoiceAssignment(mnxDynamic.staff()
                                    ? VoiceAssignment::ALL_VOICE_IN_STAFF
                                    : VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);

            s->add(dyn);
        }
    }
}

//---------------------------------------------------------
//   createOttavas
//   Create MuseScore ottava spanners from MNX ottavas.
//---------------------------------------------------------

void MnxImporter::createOttavas(const mnx::part::Measure& mnxMeasure, engraving::Measure* measure)
{
    const auto part = mnxMeasure.getEnclosingElement<mnx::Part>();
    if (const auto mnxOttavas = mnxMeasure.ottavas()) {
        for (const auto& mnxOttava : mnxOttavas.value()) {
            staff_idx_t staffIdx = muse::value(m_mnxPartStaffToStaff,
                                               std::make_pair(part->calcArrayIndex(), mnxOttava.staff()),
                                               muse::nidx);
            IF_ASSERT_FAILED(staffIdx != muse::nidx) {
                LOGE() << "staff idx not found for part " << part->pointer().to_string();
                continue;
            }
            const auto mnxEndMeasure = mnxDocument().getEntityMap().get<mnx::global::Measure>(mnxOttava.end().measure());
            Measure* endMeasure = mnxMeasureToMeasure(mnxEndMeasure.calcArrayIndex());
            const Fraction endPos = toMuseScoreFraction(mnxOttava.end().position().fraction());
            const Fraction endTick = endMeasure->tick() + endPos;
            bool endsOnBarline = false;
            if (Measure* endPlus1 = endMeasure->nextMeasure()) {
                endsOnBarline = endPlus1->tick() == endTick;
            }
            /// @todo map ottava.voice() to a relative track other than 0, if MuseScore decides to implements it.
            track_idx_t curTrackIdx = staff2track(staffIdx);

            Ottava* ottava = Factory::createOttava(m_score->dummy());
            ottava->setScore(m_score);
            ottava->setAnchor(Spanner::Anchor::SEGMENT);
            ottava->setTrack(curTrackIdx);
            ottava->setTrack2(curTrackIdx);
            ottava->setTick(measure->tick() + toMuseScoreRTick(mnxOttava.position()));
            ottava->setTick2(endTick);
            const OttavaType ottavaType = toMuseScoreOttavaType(mnxOttava.value());
            setAndStyleProperty(ottava, Pid::OTTAVA_TYPE, int(ottavaType));
            if (!endsOnBarline) {
                // ottavas in MNX include any event that starts on the endTick
                ChordRest* endCr = nullptr;
                for (track_idx_t voiceIdx = 0; voiceIdx < VOICES; voiceIdx++) {
                    ChordRest* cr = m_score->findCR(endTick, curTrackIdx + voiceIdx);
                    if (!cr) {
                        continue;
                    }
                    if (!endCr) {
                        endCr = cr;
                    } else if (endCr->endTick() > cr->endTick()) {
                        endCr = cr;
                    }
                }
                if (endCr) {
                    ottava->setEndElement(endCr);
                    ottava->setTick2(endCr->endTick());
                }
            }
            m_score->addElement(ottava);
        }
    }
}

//---------------------------------------------------------
//   createBeams
//   Apply MuseScore beam modes based on MNX beam definitions.
//---------------------------------------------------------

void MnxImporter::createBeams(const mnx::part::Measure& mnxMeasure)
{
    if (const auto beams = mnxMeasure.beams()) {
        for (const auto& beam : beams.value()) {
            const auto events = beam.events();
            for (size_t x = 0; x < events.size(); x++) {
                const auto& eventId = events[x];
                ChordRest* cr = mnxEventIdToCR(eventId);
                IF_ASSERT_FAILED(cr) {
                    LOGE() << "encountered unmapped event " << eventId << " in beam " << beam.pointer().to_string();
                    LOGE() << beam.dump(2);
                    continue;
                }
                if (events.size() == 1) {
                    cr->setBeamMode(BeamMode::NONE); // MuseScore does not have singleton beams
                } else if (x == 0) {
                    cr->setBeamMode(BeamMode::BEGIN);
                } else if (x == events.size() - 1) {
                    cr->setBeamMode(BeamMode::END);
                } else {
                    const auto mode = toMuseScoreBeamMode(mnxDocument().getEntityMap().getBeamStartLevel(eventId));
                    cr->setBeamMode(mode);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   processSequencePass2
//   Second pass over a sequence for lyrics, accidentals, slurs, ties, and rest positions.
//---------------------------------------------------------

void MnxImporter::processSequencePass2(const mnx::Sequence& sequence, Measure* measure)
{
    mnx::util::SequenceWalkHooks hooks;
    hooks.onEvent = [&](const mnx::sequence::Event& event,
                        const mnx::FractionValue&,
                        const mnx::FractionValue&, mnx::util::SequenceWalkContext& ctx) {
        ChordRest* cr = muse::value(m_mnxEventToCR, event.pointer().to_string());
        IF_ASSERT_FAILED(cr) {
            LOGE() << "event is not mapped.";
            LOGE() << event.dump(2);
            return true;
        }
        if (!ctx.inGrace) {
            createLyrics(event, cr);
        }
        if (const auto rest = event.rest(); rest && cr->isRest()) {
            createRestPosition(rest.value(), toRest(cr));
        }
        if (const auto slurs = event.slurs()) {
            for (const auto& slur : slurs.value()) {
                createSlur(slur, cr);
            }
        }
        if (const auto notes = event.notes()) {
            for (const auto& mnxNote : notes.value()) {
                Note* note = muse::value(m_mnxNoteToNote, mnxNote.pointer().to_string());
                IF_ASSERT_FAILED(note) {
                    LOGE() << "note is not mapped: " << mnxNote.pointer().to_string();
                    LOGE() << mnxNote.dump(2);
                    continue;
                }
                createAccidentals(mnxNote, note, measure);
                if (const auto ties = mnxNote.ties()) {
                    createTies(ties.value(), note);
                }
            }
        }
        if (const auto kitNotes = event.kitNotes()) {
            for (const auto& kitNote : kitNotes.value()) {
                Note* note = muse::value(m_mnxNoteToNote, kitNote.pointer().to_string());
                IF_ASSERT_FAILED(note) {
                    LOGE() << "kit note is not mapped: " << kitNote.pointer().to_string();
                    LOGE() << kitNote.dump(2);
                    continue;
                }
                if (const auto ties = kitNote.ties()) {
                    createTies(ties.value(), note);
                }
            }
        }
        return true;
    };

    mnx::util::walkSequenceContent(sequence, hooks);
}

//---------------------------------------------------------
//   importPartMeasures
//   Import all part measures with two-pass processing.
//---------------------------------------------------------

void MnxImporter::importPartMeasures()
{
    m_lyricLineUsage.clear();
    /// pass1: create ChordRests and clefs
    for (const auto& mnxPart : mnxDocument().parts()) {
        for (const auto& partMeasure : mnxPart.measures()) {
            Measure* measure = mnxMeasureToMeasure(partMeasure.calcArrayIndex());
            importSequences(mnxPart, partMeasure, measure);
            if (const auto mnxClefs = partMeasure.clefs()) {
                createClefs(mnxPart, mnxClefs.value(), measure);
            }
        }
    }
    buildLyricLineVerseMap();
    /// pass2: add accidentals, beams, dynamics, ottavas, slurs, and ties
    for (const auto& mnxPart : mnxDocument().parts()) {
        for (const auto& partMeasure : mnxPart.measures()) {
            Measure* measure = mnxMeasureToMeasure(partMeasure.calcArrayIndex());
            createDynamics(partMeasure, measure);
            createOttavas(partMeasure, measure);
            createBeams(partMeasure);
            for (const auto& sequence : partMeasure.sequences()) {
                processSequencePass2(sequence, measure);
            }
        }
    }
}
} // namespace mu::iex::mnxio
