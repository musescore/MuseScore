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
#pragma once

#include <unordered_map>
#include <map>

#include "engraving/types/propertyvalue.h"
#include "engraving/types/types.h"

#include "mnxdom.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class EngravingObject;
class EngravingItem;
class Instrument;
class Measure;
class Note;
class Part;
class Rest;
class Score;
class Staff;
class TremoloTwoChord;
class Tuplet;

enum class Pid : short;
} // namespace mu::engraving

namespace mu::iex::mnxio {
class MnxImporter
{
public:
    MnxImporter(engraving::Score* s, mnx::Document&& doc)
        : m_score(s), m_mnxDocument(std::move(doc)) {}
    void importMnx();

    const mnx::Document& mnxDocument() const { return m_mnxDocument; }

    engraving::Score* score() const { return m_score; }

private:
    using GraceNeighborsMap = std::unordered_map<std::string,
                                                 std::pair<engraving::ChordRest*, engraving::ChordRest*> >;

    // settings
    void importSettings();

    // parts
    void importParts();
    void createStaff(engraving::Part* part, const mnx::Part& mnxPart, int staffNum);

    // brackets and barlines
    void importBrackets();

    // global measures
    void importGlobalMeasures();
    void buildLyricLineVerseMap();
    void createKeySig(engraving::Measure* measure, int keyFifths); // positive = sharps; negative = flats
    void createTimeSig(engraving::Measure* measure, const mnx::TimeSignature& timeSig);
    void setBarline(engraving::Measure* measure, const mnx::global::Barline& barline);
    void createVolta(engraving::Measure* measure, const mnx::global::Ending& ending);
    void createJumpOrMarker(engraving::Measure* measure, const mnx::FractionValue& location, std::variant<engraving::JumpType,
                                                                                                          engraving::MarkerType> type,
                            const std::optional<std::string> glyphName = std::nullopt);
    void createTempoMark(engraving::Measure* measure, const mnx::global::Tempo& tempo);

    // part measures
    void importPartMeasures();
    void importSequences(const mnx::Part& mnxPart, const mnx::part::Measure& partMeasure, engraving::Measure* measure);
    bool importNonGraceEvents(const mnx::Sequence& sequence, engraving::Measure* measure, engraving::track_idx_t curTrackIdx,
                              GraceNeighborsMap& graceNeighbors);
    void updateLyricLineUsageForEvent(const mnx::sequence::Event& event, const mnx::Sequence& sequence, engraving::Measure* measure,
                                      engraving::track_idx_t curTrackIdx, const mnx::FractionValue& startTick);
    void importGraceEvents(const mnx::Sequence& sequence, engraving::Measure* measure, engraving::track_idx_t curTrackIdx,
                           const GraceNeighborsMap& graceNeighbors);
    engraving::ChordRest* importEvent(const mnx::sequence::Event& event, engraving::track_idx_t, engraving::Measure* measure,
                                      const mnx::FractionValue& startTick, const std::stack<engraving::Tuplet*>& activeTuplets,
                                      engraving::TremoloTwoChord* activeTremolo);
    engraving::Tuplet* createTuplet(const mnx::sequence::Tuplet& mnxTuplet, engraving::Measure* measure,
                                    engraving::track_idx_t curTrackIdx);
    void createTremolo(const mnx::sequence::MultiNoteTremolo& mnxTremolo, engraving::Measure* measure, engraving::track_idx_t curTrackIdx,
                       const mnx::FractionValue& startTick, const mnx::FractionValue& endTick);
    void processSequencePass2(const mnx::Sequence& sequence, engraving::Measure* measure);
    void createSlur(const mnx::sequence::Slur& mnxSlur, engraving::ChordRest* startCR);
    void createLyrics(const mnx::sequence::Event& mnxEvent, engraving::ChordRest* cr);
    void createTies(const mnx::Array<mnx::sequence::Tie>& ties, engraving::Note* startNote);
    void createAccidentals(const mnx::sequence::Note& mnxNote, engraving::Note* note, engraving::Measure* measure);
    void createRestPosition(const mnx::sequence::Rest& mnxRest, engraving::Rest* rest);
    engraving::Rest* emitGapRest(engraving::Measure* measure, engraving::track_idx_t curTrackIdx, const mnx::FractionValue& startTick,
                                 const mnx::FractionValue& duration, engraving::Tuplet* tupletToAdd);
    engraving::Note* createNote(const mnx::sequence::Note& mnxNote, engraving::Chord* chord, engraving::Staff* baseStaff,
                                const engraving::Fraction& tick, int ottavaDisplacement, engraving::track_idx_t curTrackIdx);
    void createClefs(const mnx::Part& mnxPart, const mnx::Array<mnx::part::PositionedClef>& mnxClefs, engraving::Measure* measure);
    void createOttavas(const mnx::part::Measure& mnxMeasure, engraving::Measure* measure);
    void createBeams(const mnx::part::Measure& mnxMeasure);
    void createDynamics(const mnx::part::Measure& mnxMeasure, engraving::Measure* measure);

    // markings
    void importMarkings(const mnx::sequence::Event& mnxEvent, engraving::ChordRest* cr);
    void importAccent(const mnx::sequence::Accent& accent, engraving::ChordRest* cr);
    void importBreath(const mnx::sequence::BreathMark& breath, engraving::ChordRest* cr);
    void importSoftAccent(const mnx::sequence::SoftAccent& softAccent, engraving::ChordRest* cr);
    void importSpiccato(const mnx::sequence::Spiccato& spiccato, engraving::ChordRest* cr);
    void importStaccatissimo(const mnx::sequence::Staccatissimo& staccatissimo, engraving::ChordRest* cr);
    void importStaccato(const mnx::sequence::Staccato& staccato, engraving::ChordRest* cr);
    void importStress(const mnx::sequence::Stress& stress, engraving::ChordRest* cr);
    void importStrongAccent(const mnx::sequence::StrongAccent& strongAccent, engraving::ChordRest* cr);
    void importTenuto(const mnx::sequence::Tenuto& tenuto, engraving::ChordRest* cr);
    void importTremolo(const mnx::sequence::SingleNoteTremolo& tremolo, engraving::ChordRest* cr);
    void importUnstress(const mnx::sequence::Unstress& unstress, engraving::ChordRest* cr);

    // utility funcs
    engraving::staff_idx_t mnxPartStaffToStaffIdx(const mnx::Part& mnxPart, int staffNum);
    std::optional<engraving::staff_idx_t> mnxLayoutStaffToStaffIdx(const mnx::layout::Staff& mnxStaff); // returns the first part corresponding part staff found
    engraving::Measure* mnxMeasureToMeasure(const size_t mnxMeasIdx);
    engraving::ChordRest* mnxEventIdToCR(const std::string& eventId);
    engraving::Note* mnxNoteIdToNote(const std::string& noteId);
    static void setAndStyleProperty(engraving::EngravingObject* e, engraving::Pid id, engraving::PropertyValue v,
                                    bool inheritStyle = false);
    engraving::Fraction mnxMeasurePosToTick(const mnx::MeasureRhythmicPosition& measPos);

    // ordered map avoids need for hash on std::pair
    std::map<std::pair<size_t, int>, engraving::staff_idx_t> m_mnxPartStaffToStaff;
    std::unordered_map<engraving::staff_idx_t, size_t> m_StaffToMnxPart;
    std::unordered_map<size_t, engraving::Fraction> m_mnxMeasToTick;
    std::unordered_map<engraving::staff_idx_t,
                       std::unordered_map<std::string, std::pair<engraving::Fraction, engraving::Fraction> > > m_lyricLineUsage;
    std::unordered_map<engraving::staff_idx_t, std::unordered_map<std::string, int> > m_lyricLineToVerse;
    // barline span tracking
    std::vector<std::pair<engraving::staff_idx_t, engraving::staff_idx_t> > m_barlineSpans;
    std::unordered_map<engraving::staff_idx_t, size_t> m_staffToSpan;
    // event tracking
    std::unordered_map<std::string, engraving::ChordRest*> m_mnxEventToCR; // key is json_pointer, since event.id() is optional.
    std::unordered_map<std::string, engraving::Note*> m_mnxNoteToNote; // key is json_pointer, since event.id() is optional.
    std::map<std::pair<size_t, std::string>, int> m_mnxKitComponentToMidi;

    bool m_useBeams{}; // if true, only events in mnx beams arrays should be beamed.
    bool m_useAccidentalDisplay{}; // if true, all accidentals are force hide or show.

    engraving::Score* m_score{};
    mnx::Document m_mnxDocument;
};
} // namespace mu::iex::mnxio
