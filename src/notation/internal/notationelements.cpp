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
#include "notationelements.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/segment.h"

#include "searchcommandsparser.h"

#include "log.h"

using namespace muse;
using namespace mu::notation;

NotationElements::NotationElements(IGetScore* getScore)
    : m_getScore(getScore)
{
}

mu::engraving::Score* NotationElements::msScore() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }
    return m_getScore->score();
}

std::vector<EngravingItem*> NotationElements::search(const QString& searchText) const
{
    SearchCommandsParser commandsParser;

    SearchCommandsParser::SearchData searchData = commandsParser.parse(searchText);
    if (!searchData.isValid()) {
        return {};
    }

    switch (searchData.type) {
    case SearchCommandsParser::SearchData::Type::Invalid:
        return {};
    case SearchCommandsParser::SearchData::Type::RehearsalMark: {
        String name = searchData.rehearsalMark();
        mu::engraving::RehearsalMark* rehearsalMark = this->rehearsalMark(name);
        if (rehearsalMark) {
            return { rehearsalMark };
        }
        break;
    }
    case SearchCommandsParser::SearchData::Type::Measure: {
        int measureIndex = searchData.measureIndex() - 1;
        mu::engraving::Measure* measure = this->measure(measureIndex);
        if (measure) {
            return { measure };
        }
        break;
    }
    case SearchCommandsParser::SearchData::Type::MeasureRange: {
        auto [startMeasureIndex, endMeasureIndex] = searchData.measureRange();
        mu::engraving::Measure* startMeasure = this->measure(startMeasureIndex - 1);
        mu::engraving::Measure* endMeasure = this->measure(endMeasureIndex - 1);
        std::vector<EngravingItem*> result;
        if (startMeasure) {
            result.push_back(startMeasure);
        }
        if (endMeasure && endMeasure != startMeasure) {
            result.push_back(endMeasure);
        }
        return result;
    }
    case SearchCommandsParser::SearchData::Type::Page: {
        int pageIndex = searchData.pageIndex() - 1;
        mu::engraving::Page* page = this->page(pageIndex);
        if (page) {
            return { page };
        }
        break;
    }
    }

    return {};
}

std::vector<EngravingItem*> NotationElements::elements(const FilterElementsOptions& elementsOptions) const
{
    std::vector<EngravingItem*> result;

    const FilterElementsOptions* elementsFilterOptions = dynamic_cast<const FilterElementsOptions*>(&elementsOptions);

    if (!elementsFilterOptions) {
        return allScoreElements();
    }

    // todo: add check for range search

    const FilterNotesOptions* noteOptions = dynamic_cast<const FilterNotesOptions*>(elementsFilterOptions);
    if (noteOptions) {
        result = filterNotes(noteOptions);
    } else {
        result = filterElements(elementsFilterOptions);
    }

    return result;
}

mu::engraving::RehearsalMark* NotationElements::rehearsalMark(const String& name) const
{
    String qname = name.toLower();

    for (mu::engraving::Segment* segment = score()->firstSegment(mu::engraving::SegmentType::ChordRest); segment;
         segment = segment->next1(mu::engraving::SegmentType::ChordRest)) {
        for (EngravingItem* element: segment->annotations()) {
            if (!element->isRehearsalMark()) {
                continue;
            }

            mu::engraving::RehearsalMark* rehearsalMark = toRehearsalMark(element);
            String rehearsalMarkName = rehearsalMark->plainText().toLower();
            if (rehearsalMarkName.startsWith(qname)) {
                return rehearsalMark;
            }
        }
    }

    return nullptr;
}

mu::engraving::Measure* NotationElements::measure(const int measureIndex) const
{
    return score()->crMeasure(measureIndex);
}

const PageList& NotationElements::pages() const
{
    return score()->pages();
}

const Page* NotationElements::pageByPoint(const PointF& point) const
{
    return score()->searchPage(point);
}

mu::engraving::Page* NotationElements::page(const int pageIndex) const
{
    if (pageIndex < 0 || size_t(pageIndex) >= score()->pages().size()) {
        return nullptr;
    }

    return score()->pages().at(pageIndex);
}

std::vector<EngravingItem*> NotationElements::allScoreElements() const
{
    std::vector<EngravingItem*> result;
    for (mu::engraving::Page* page : score()->pages()) {
        for (EngravingItem* element: page->elements()) {
            result.push_back(element);
        }
    }

    return result;
}

std::vector<EngravingItem*> NotationElements::filterElements(const FilterElementsOptions* elementsOptions) const
{
    ElementPattern pattern = constructElementPattern(elementsOptions);

    score()->scanElements([&](EngravingItem* item) { mu::engraving::Score::collectMatch(&pattern, item); });

    std::vector<EngravingItem*> result;
    for (EngravingItem* element: pattern.el) {
        result.push_back(element);
    }

    return result;
}

std::vector<EngravingItem*> NotationElements::filterNotes(const FilterNotesOptions* notesOptions) const
{
    mu::engraving::NotePattern pattern = constructNotePattern(notesOptions);

    score()->scanElements([&](EngravingItem* item) { mu::engraving::Score::collectNoteMatch(&pattern, item); });

    std::vector<EngravingItem*> result;
    for (EngravingItem* element: pattern.el) {
        result.push_back(element);
    }

    return result;
}

mu::engraving::Score* NotationElements::score() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }

    return m_getScore->score();
}

ElementPattern NotationElements::constructElementPattern(const FilterElementsOptions* elementOptions) const
{
    mu::engraving::ElementPattern pattern;
    pattern.type = static_cast<int>(elementOptions->elementType);
    pattern.subtype = elementOptions->subtype;
    pattern.subtypeValid = elementOptions->bySubtype;
    pattern.staffStart = elementOptions->staffStart;
    pattern.staffEnd = elementOptions->staffEnd;
    pattern.voice   = elementOptions->voice;
    pattern.system  = elementOptions->system;
    pattern.durationTicks = elementOptions->durationTicks;
    pattern.beat = elementOptions->beat;
    pattern.measure = elementOptions->measure;

    return pattern;
}

mu::engraving::NotePattern NotationElements::constructNotePattern(const FilterNotesOptions* notesOptions) const
{
    mu::engraving::NotePattern pattern;
    pattern.pitch = notesOptions->pitch;
    pattern.string = notesOptions->string;
    pattern.tpc = notesOptions->tpc;
    pattern.notehead = notesOptions->notehead;
    pattern.durationType = notesOptions->durationType;
    pattern.durationTicks = notesOptions->durationTicks;
    pattern.type = notesOptions->noteType;
    pattern.staffStart = notesOptions->staffStart;
    pattern.staffEnd = notesOptions->staffEnd;
    pattern.voice = notesOptions->voice;
    pattern.system = notesOptions->system;
    pattern.beat = notesOptions->beat;
    pattern.measure = notesOptions->measure;

    return pattern;
}
