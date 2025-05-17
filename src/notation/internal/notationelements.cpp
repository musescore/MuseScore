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

#include "log.h"
#include "searchcommandsparser.h"

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

EngravingItem* NotationElements::search(const std::string& searchText) const
{
    SearchCommandsParser commandsParser;

    SearchCommandsParser::SearchData searchData = commandsParser.parse(searchText);
    if (searchData.isValid()) {
        switch (searchData.elementType) {
        case ElementType::REHEARSAL_MARK: {
            return rehearsalMark(searchData.value.toString().toStdString());
        }
        case ElementType::MEASURE: {
            //!NOTE: the measure numbering in the service starts from zero
            int measureIndex = searchData.value.toInt() - 1;
            return measure(measureIndex);
        }
        case ElementType::PAGE: {
            //!NOTE: the page numbering in the service starts from zero
            int pageIndex = searchData.value.toInt() - 1;
            return page(pageIndex);
        }
        default:
            return nullptr;
        }
    }

    return nullptr;
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

mu::engraving::RehearsalMark* NotationElements::rehearsalMark(const std::string& name) const
{
    QString qname = QString::fromStdString(name).toLower();

    for (mu::engraving::Segment* segment = score()->firstSegment(mu::engraving::SegmentType::ChordRest); segment;
         segment = segment->next1(mu::engraving::SegmentType::ChordRest)) {
        for (EngravingItem* element: segment->annotations()) {
            if (element->type() != ElementType::REHEARSAL_MARK) {
                continue;
            }

            mu::engraving::RehearsalMark* rehearsalMark = static_cast<mu::engraving::RehearsalMark*>(element);
            QString rehearsalMarkName = rehearsalMark->plainText().toQString().toLower();
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

PageList NotationElements::pages() const
{
    PageList result;
    for (const Page* page : score()->pages()) {
        result.push_back(page);
    }

    return result;
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
    ElementPattern* pattern = constructElementPattern(elementsOptions);

    score()->scanElements(pattern, mu::engraving::Score::collectMatch);

    std::vector<EngravingItem*> result;
    for (EngravingItem* element: pattern->el) {
        result.push_back(element);
    }

    return result;
}

std::vector<EngravingItem*> NotationElements::filterNotes(const FilterNotesOptions* notesOptions) const
{
    mu::engraving::NotePattern* pattern = constructNotePattern(notesOptions);

    score()->scanElements(pattern, mu::engraving::Score::collectNoteMatch);

    std::vector<EngravingItem*> result;
    for (EngravingItem* element: pattern->el) {
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

ElementPattern* NotationElements::constructElementPattern(const FilterElementsOptions* elementOptions) const
{
    mu::engraving::ElementPattern* pattern = new mu::engraving::ElementPattern();
    pattern->type = static_cast<int>(elementOptions->elementType);
    pattern->subtype = elementOptions->subtype;
    pattern->subtypeValid = elementOptions->bySubtype;
    pattern->staffStart = elementOptions->staffStart;
    pattern->staffEnd = elementOptions->staffEnd;
    pattern->voice   = elementOptions->voice;
    pattern->system  = elementOptions->system;
    pattern->durationTicks = elementOptions->durationTicks;
    pattern->beat = elementOptions->beat;
    pattern->measure = elementOptions->measure;

    return pattern;
}

mu::engraving::NotePattern* NotationElements::constructNotePattern(const FilterNotesOptions* notesOptions) const
{
    mu::engraving::NotePattern* pattern = new mu::engraving::NotePattern();
    pattern->pitch = notesOptions->pitch;
    pattern->string = notesOptions->string;
    pattern->tpc = notesOptions->tpc;
    pattern->notehead = notesOptions->notehead;
    pattern->durationType = notesOptions->durationType;
    pattern->durationTicks = notesOptions->durationTicks;
    pattern->type = notesOptions->noteType;
    pattern->staffStart = notesOptions->staffStart;
    pattern->staffEnd = notesOptions->staffEnd;
    pattern->voice = notesOptions->voice;
    pattern->system = notesOptions->system;
    pattern->beat = notesOptions->beat;
    pattern->measure = notesOptions->measure;
    pattern->chordIndex = notesOptions->chordIndex;

    return pattern;
}
