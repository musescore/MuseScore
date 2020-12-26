//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationelements.h"

#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"

#include "log.h"
#include "searchcommandsparser.h"

using namespace mu::notation;

NotationElements::NotationElements(IGetScore* getScore)
    : m_getScore(getScore)
{
}

Ms::Score* NotationElements::msScore() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }
    return m_getScore->score();
}

Element* NotationElements::search(const std::string& searchText) const
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

std::vector<Element*> NotationElements::elements(const FilterElementsOptions& elementsOptions) const
{
    std::vector<Element*> result;

    const FilterElementsOptions* elementsFilterOptions = dynamic_cast<const FilterElementsOptions*>(&elementsOptions);

    if (!elementsFilterOptions || !elementsFilterOptions->isValid()) {
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

Ms::RehearsalMark* NotationElements::rehearsalMark(const std::string& name) const
{
    QString qname = QString::fromStdString(name).toLower();

    for (Ms::Segment* segment = score()->firstSegment(Ms::SegmentType::ChordRest); segment;
         segment = segment->next1(Ms::SegmentType::ChordRest)) {
        for (Element* element: segment->annotations()) {
            if (element->type() != ElementType::REHEARSAL_MARK) {
                continue;
            }

            Ms::RehearsalMark* rehearsalMark = static_cast<Ms::RehearsalMark*>(element);
            QString rehearsalMarkName = rehearsalMark->plainText().toLower();
            if (rehearsalMarkName.startsWith(qname)) {
                return rehearsalMark;
            }
        }
    }

    return nullptr;
}

Ms::Measure* NotationElements::measure(const int measureIndex) const
{
    return score()->crMeasure(measureIndex);
}

PageList NotationElements::pages() const
{
    PageList result;
    for (const Page* page: score()->pages()) {
        result.push_back(page);
    }

    return result;
}

Ms::Page* NotationElements::page(const int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= score()->pages().size()) {
        return nullptr;
    }

    return score()->pages().at(pageIndex);
}

std::vector<Element*> NotationElements::allScoreElements() const
{
    std::vector<Element*> result;
    for (Ms::Page* page : score()->pages()) {
        for (Element* element: page->elements()) {
            result.push_back(element);
        }
    }

    return result;
}

std::vector<Element*> NotationElements::filterElements(const FilterElementsOptions* elementsOptions) const
{
    ElementPattern* pattern = constructElementPattern(elementsOptions);

    score()->scanElements(pattern, Ms::Score::collectMatch);

    std::vector<Element*> result;
    for (Element* element: pattern->el) {
        result.push_back(element);
    }

    return result;
}

std::vector<Element*> NotationElements::filterNotes(const FilterNotesOptions* notesOptions) const
{
    Ms::NotePattern* pattern = constructNotePattern(notesOptions);

    score()->scanElements(pattern, Ms::Score::collectNoteMatch);

    std::vector<Element*> result;
    for (Element* element: pattern->el) {
        result.push_back(element);
    }

    return result;
}

Ms::Score* NotationElements::score() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }

    return m_getScore->score();
}

ElementPattern* NotationElements::constructElementPattern(const FilterElementsOptions* elementOptions) const
{
    ElementPattern* pattern = new ElementPattern;
    pattern->type = static_cast<int>(elementOptions->elementType);
    pattern->subtype = elementOptions->subtype;
    pattern->subtypeValid = elementOptions->bySubtype;
    pattern->staffStart = elementOptions->staffStart;
    pattern->staffEnd = elementOptions->staffEnd;
    pattern->voice   = elementOptions->voice;
    pattern->system  = elementOptions->system;
    pattern->durationTicks = elementOptions->durationTicks;

    return pattern;
}

Ms::NotePattern* NotationElements::constructNotePattern(const FilterNotesOptions* notesOptions) const
{
    Ms::NotePattern* pattern = new Ms::NotePattern();
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

    return pattern;
}
