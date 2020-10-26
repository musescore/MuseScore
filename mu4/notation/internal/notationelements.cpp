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

Ms::Score* NotationElements::score() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }

    return m_getScore->score();
}
