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


// #include "engraving/engravingerrors.h"
#include "style/style.h"
#include "engraving/dom/tuplet.h"

#include "musx/musx.h"

namespace mu::engraving {
class InstrumentTemplate;
class Part;
class Score;
class Staff;
}

namespace mu::iex::finale {
class FinaleLogger;

struct ReadableTuplet
{
    engraving::Fraction absBegin;
    engraving::Fraction absDuration;
    engraving::Fraction absEnd;
    std::shared_ptr<const musx::dom::details::TupletDef> musxTuplet = nullptr; // actual tuplet object. used for writing properties
    engraving::Tuplet* scoreTuplet = nullptr; // to be created tuplet object.
    int layer{}; // for nested tuplets. 0 = outermost
};

class EnigmaXmlImporter
{
public:
    EnigmaXmlImporter(engraving::Score* score, const std::shared_ptr<musx::dom::Document>& doc, const std::shared_ptr<FinaleLogger>& logger)
        : m_score(score), m_doc(doc), m_logger(logger) {}

    void import();

    const engraving::Score* score() const { return m_score; }
    std::shared_ptr<musx::dom::Document> musxDocument() const { return m_doc; }

    std::shared_ptr<FinaleLogger> logger() const { return m_logger; }

private:
    void mapLayers();
    void importParts();
    void importMeasures();
    void importBrackets();
    void importStyles(engraving::MStyle& style, musx::dom::Cmper partId);

    bool processEntryInfo(/*const std::shared_ptr<const musx::dom::EntryInfo>*/ musx::dom::EntryInfoPtr entryInfo, engraving::track_idx_t curTrackIdx,
                          engraving::Segment* segment, std::vector<ReadableTuplet>& tupletMap, size_t& lastAddedTupletIndex);

    engraving::ChordRest* importEntry(musx::dom::EntryInfoPtr entryInfo, engraving::Segment* segment,
                                      engraving::track_idx_t curTrackIdx);

    void fillWithInvisibleRests(engraving::Fraction startTick, engraving::track_idx_t curTrackIdx, engraving::Fraction lengthToFill,
                                std::vector<ReadableTuplet> tupletMap);

    engraving::Staff* createStaff(engraving::Part* part, const std::shared_ptr<const musx::dom::others::Staff> musxStaff,
                                  const engraving::InstrumentTemplate* it = nullptr);

    std::unordered_map<int, engraving::track_idx_t> mapFinaleVoices(const std::map<musx::dom::LayerIndex, bool>& finaleVoiceMap,
                                                         musx::dom::InstCmper curStaff, musx::dom::MeasCmper curMeas) const;
    engraving::Score* m_score;
    const std::shared_ptr<musx::dom::Document> m_doc;
    const std::shared_ptr<FinaleLogger> m_logger;

    std::unordered_map<QString, std::vector<musx::dom::InstCmper>> m_part2Inst;
    std::unordered_map<musx::dom::InstCmper, QString> m_inst2Part;
    std::unordered_map<engraving::staff_idx_t, musx::dom::InstCmper> m_staff2Inst;
    std::unordered_map<musx::dom::InstCmper, engraving::staff_idx_t> m_inst2Staff;
    std::unordered_map<musx::dom::LayerIndex, engraving::track_idx_t> m_layer2Voice;
    std::unordered_set<musx::dom::LayerIndex> m_layerForceStems;
};

}
