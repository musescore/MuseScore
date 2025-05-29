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

#pragma once

#include <unordered_map>
#include <memory>

#include "style/style.h"
#include "engraving/dom/tuplet.h"

#include "musx/musx.h"

#include "importfinalelogger.h"

namespace mu::engraving {
class InstrumentTemplate;
class Part;
class Score;
class Staff;
}

namespace mu::iex::finale {

struct ReadableTuplet {
    engraving::Fraction startTick;
    engraving::Fraction endTick;
    std::shared_ptr<const musx::dom::details::TupletDef> musxTuplet = nullptr; // actual tuplet object. used for writing properties
    engraving::Tuplet* scoreTuplet = nullptr; // to be created tuplet object.
    int layer = 0; // for nested tuplets. 0 = outermost
};

class FinaleParser
{
public:
    FinaleParser(engraving::Score* score, const std::shared_ptr<musx::dom::Document>& doc, FinaleLoggerPtr& logger)
        : m_score(score), m_doc(doc), m_logger(logger) {}

    void parse();

    const engraving::Score* score() const { return m_score; }
    std::shared_ptr<musx::dom::Document> musxDocument() const { return m_doc; }

    FinaleLoggerPtr logger() const { return m_logger; }

private:
    // scoremap
    void importParts();
    void importBrackets();
    void importMeasures();
    void importPageLayout();
    void importStaffItems();

    engraving::Staff* createStaff(engraving::Part* part, const std::shared_ptr<const musx::dom::others::Staff> musxStaff,
                                  const engraving::InstrumentTemplate* it = nullptr);
    void importClefs(const std::shared_ptr<musx::dom::others::InstrumentUsed>& musxScrollViewItem,
                     const std::shared_ptr<musx::dom::others::Measure>& musxMeasure,
                     engraving::Measure* measure, engraving::staff_idx_t curStaffIdx,
                     musx::dom::ClefIndex& musxCurrClef);
    // entries
    /// @todo create readContext struct with tick, segment, track, measure, etc
    void mapLayers();
    void importEntries();

    std::unordered_map<int, engraving::track_idx_t> mapFinaleVoices(const std::map<musx::dom::LayerIndex, bool>& finaleVoiceMap,
                                                         musx::dom::InstCmper curStaff, musx::dom::MeasCmper curMeas) const;
    bool processEntryInfo(musx::dom::EntryInfoPtr entryInfo, engraving::track_idx_t curTrackIdx, engraving::Measure* measure,
                          std::vector<ReadableTuplet>& tupletMap);
    bool processBeams(musx::dom::EntryInfoPtr entryInfoPtr, engraving::track_idx_t curTrackIdx, engraving::Measure* measure);

    // styles
    void importStyles(engraving::MStyle& style, musx::dom::Cmper partId);

    engraving::Score* m_score;
    const std::shared_ptr<musx::dom::Document> m_doc;
    FinaleLoggerPtr m_logger;
    const musx::dom::Cmper m_currentMusxPartId = musx::dom::SCORE_PARTID; // eventually this may be changed per excerpt/linked part

    std::unordered_map<QString, std::vector<musx::dom::InstCmper>> m_part2Inst;
    std::unordered_map<musx::dom::InstCmper, QString> m_inst2Part;
    std::unordered_map<engraving::staff_idx_t, musx::dom::InstCmper> m_staff2Inst;
    std::unordered_map<musx::dom::InstCmper, engraving::staff_idx_t> m_inst2Staff;
    std::unordered_map<musx::dom::MeasCmper, engraving::Fraction> m_meas2Tick;
    std::map<engraving::Fraction, musx::dom::MeasCmper> m_tick2Meas; // use std::map to avoid need for Fraction hash function
    std::unordered_map<musx::dom::LayerIndex, engraving::voice_idx_t> m_layer2Voice;
    std::unordered_set<musx::dom::LayerIndex> m_layerForceStems;
    std::map<musx::dom::NoteInfoPtr, engraving::Note*> m_noteInfoPtr2Note; // use std::map to avoid need for NoteInfoPtr hash function
};

}
