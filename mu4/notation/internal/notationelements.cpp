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

Ms::Element* NotationElements::search(const std::string& searchCommand) const
{
    SearchCommandsParser commandsParser;

    SearchCommandsParser::SearchData searchData = commandsParser.parse(searchCommand);
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

std::vector<Ms::Element*> NotationElements::searchSimilar(const SearchElementOptions* elementOptions) const
{
    std::vector<Ms::Element*> result;

    // todo: add check for range search

    bool isNote = dynamic_cast<const SearchNoteOptions*>(elementOptions) != nullptr;
    if (isNote) {
        result = searchNotes(elementOptions);
    } else {
        result = searchElements(elementOptions);
    }

    return result;
}

Ms::RehearsalMark* NotationElements::rehearsalMark(const std::string& name) const
{
    QString qname = QString::fromStdString(name).toLower();

    for (Ms::Segment* segment = score()->firstSegment(Ms::SegmentType::ChordRest); segment;
         segment = segment->next1(Ms::SegmentType::ChordRest)) {
        for (Ms::Element* element: segment->annotations()) {
            if (element->type() != Ms::ElementType::REHEARSAL_MARK) {
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

Ms::Page* NotationElements::page(const int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= score()->pages().size()) {
        return nullptr;
    }

    return score()->pages().at(pageIndex);
}

std::vector<Ms::Element*> NotationElements::searchElements(const SearchElementOptions* elementOptions) const
{
    Ms::ElementPattern* pattern = constructElementPattern(elementOptions);

    score()->scanElements(pattern, Ms::Score::collectMatch);

    std::vector<Element*> result;
    for (Element* element: pattern->el) {
        result.push_back(element);
    }

    return result;
}

std::vector<Ms::Element*> NotationElements::searchNotes(const SearchElementOptions* elementOptions) const
{
    Ms::NotePattern* pattern = constructNotePattern(elementOptions);

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

ElementPattern* NotationElements::constructElementPattern(const SearchElementOptions* elementOptions) const
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

Ms::NotePattern* NotationElements::constructNotePattern(const SearchElementOptions* elementOptions) const
{
    const SearchNoteOptions* noteOptions = dynamic_cast<const SearchNoteOptions*>(elementOptions);

    Ms::NotePattern* pattern = new Ms::NotePattern();
    pattern->pitch = noteOptions->pitch;
    pattern->string = noteOptions->string;
    pattern->tpc = noteOptions->tpc;
    pattern->notehead = noteOptions->notehead;
    pattern->durationType = noteOptions->durationType;
    pattern->durationTicks = noteOptions->durationTicks;
    pattern->type = noteOptions->noteType;
    pattern->staffStart = noteOptions->staffStart;
    pattern->staffEnd = noteOptions->staffEnd;
    pattern->voice = noteOptions->voice;
    pattern->system = noteOptions->system;

    return pattern;
}
