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
#include <unordered_set>
#include <vector>

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif
#include "types/ret.h"

#include "engraving/types/types.h"

namespace mu::engraving {
class ChordRest;
class EID;
class EngravingItem;
class EngravingObject;
class GraceNotesGroup;
class Instrument;
class Measure;
class Note;
class Part;
class Score;
class Staff;
class TremoloTwoChord;
class Tuplet;
} // namespace mu::engraving

namespace mu::iex::mnxio {
class MnxExporter
{
public:
    MnxExporter(engraving::Score* s, bool exportBeams = true, bool exportRestPositions = false)
        : m_score(s), m_exportBeams(exportBeams), m_exportRestPositions(exportRestPositions) {}
    muse::Ret exportMnx();

    const mnx::Document& mnxDocument() const
    { return m_mnxDocument; }
    const std::unordered_map<engraving::staff_idx_t, std::pair<size_t, int> >& staffToPartStaff() const
    { return m_staffToPartStaff; }

    // utility functions
    engraving::EID getOrAssignEID(engraving::EngravingObject* item);
    std::optional<mnx::sequence::Event> mnxEventFromCR(const engraving::ChordRest* cr);
    size_t mnxMeasureIndexFromMeasure(const engraving::Measure* measure) const;
    std::pair<size_t, int> mnxPartStaffFromStaffIdx(engraving::staff_idx_t staffIdx) const;

private:
    enum class ContentContext {
        Sequence,
        Grace,
        Tuplet,
        Tremolo
    };

    struct ExportContext {
        ExportContext(const engraving::Part* partIn, const engraving::Measure* measureIn,
                      mnx::part::Measure mnxMeasureIn, engraving::staff_idx_t staffIdxIn,
                      engraving::voice_idx_t voiceIn, int mnxPartStaffIn)
            : part(partIn),
            measure(measureIn),
            mnxMeasure(mnxMeasureIn),
            staffIdx(staffIdxIn),
            voice(voiceIn),
            mnxPartStaff(mnxPartStaffIn)
        {
        }

        const engraving::Part* part{};
        const engraving::Measure* measure{};
        mnx::part::Measure mnxMeasure;
        engraving::staff_idx_t staffIdx{};
        engraving::voice_idx_t voice{};
        int mnxPartStaff{}; // 1-based staff number within part.
        std::vector<const engraving::Tuplet*> tupletStack;
        std::unordered_set<const engraving::ChordRest*> graceBeforeEmitted;
        std::unordered_set<const engraving::ChordRest*> graceAfterEmitted;
    };

    void createGlobal();
    bool createParts();
    void createLayout(const std::vector<engraving::Staff*>& staves, const std::string& layoutId);
    void createSequences(const engraving::Part* part, const engraving::Measure* measure, mnx::part::Measure& mnxMeasure);
    void appendContent(mnx::ContentArray content, ExportContext& ctx, const std::vector<engraving::ChordRest*>& chordRests,
                       ContentContext context);
    void appendGrace(mnx::ContentArray content, ExportContext& ctx, engraving::GraceNotesGroup& graceNotes);
    void createBeam(ExportContext& ctx, engraving::ChordRest* chordRest);
    size_t appendTuplet(mnx::ContentArray content, ExportContext& ctx, const std::vector<engraving::ChordRest*>& chordRests, size_t idx,
                        engraving::ChordRest* chordRest, const engraving::Tuplet* tuplet);
    size_t appendTremolo(mnx::ContentArray content, ExportContext& ctx, const std::vector<engraving::ChordRest*>& chordRests, size_t idx,
                         engraving::ChordRest* chordRest);
    bool appendEvent(mnx::ContentArray content, ExportContext& ctx, engraving::ChordRest* chordRest);
    bool createRest(mnx::sequence::Event& mnxEvent, engraving::ChordRest* chordRest);
    bool createNotes(mnx::sequence::Event& mnxEvent, engraving::ChordRest* chordRest);
    void createTies(mnx::sequence::NoteBase& mnxNote, engraving::Note* note);
    const engraving::Tuplet* findTopTuplet(engraving::ChordRest* chordRest, const ExportContext& ctx) const;
    void exportDrumsetKit(const engraving::Part* part, const engraving::Instrument* instrument, mnx::Part& mnxPart);

    void exportSpanners();

    engraving::Score* m_score{};
    mnx::Document m_mnxDocument;

    // entity tracking
    std::unordered_map<const engraving::Measure*, size_t> m_measToMnxMeas;
    std::unordered_map<const engraving::ChordRest*, mnx::json_pointer> m_crToMnxEvent;
    std::unordered_map<engraving::staff_idx_t, std::pair<size_t, int> > m_staffToPartStaff;
    std::vector<engraving::Staff*> m_exportedStaves;
    std::set<std::string> m_lyricLineIds; // this could be (ordered) map if we ever support lyric line metadata.

    bool m_exportBeams { true };
    bool m_exportRestPositions { false };
};
} // namespace mu::iex::mnxio
