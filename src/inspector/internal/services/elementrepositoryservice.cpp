/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "elementrepositoryservice.h"

#include "chord.h"
#include "stem.h"
#include "hook.h"
#include "beam.h"
#include "glissando.h"
#include "hairpin.h"
#include "volta.h"
#include "staff.h"
#include "layoutbreak.h"
#include "pedal.h"
#include "tremolo.h"
#include "bracket.h"
#include "bracketItem.h"
#include "durationtype.h"
#include "stafftype.h"
#include "mscore.h"

#include "log.h"
#include "types/texttypes.h"

using namespace mu::inspector;
using namespace mu::notation;

ElementRepositoryService::ElementRepositoryService(QObject* parent)
    : QObject(parent)
{
}

QObject* ElementRepositoryService::getQObject()
{
    return this;
}

bool ElementRepositoryService::needUpdateElementList(const QList<Ms::EngravingItem*>& newRawElementList,
                                                     SelectionState selectionState) const
{
    return m_rawElementList != newRawElementList || m_selectionState != selectionState;
}

void ElementRepositoryService::updateElementList(const QList<Ms::EngravingItem*>& newRawElementList, SelectionState selectionState)
{
    if (!needUpdateElementList(newRawElementList, selectionState)) {
        return;
    }

    m_exposedElementList = exposeRawElements(newRawElementList);
    m_rawElementList = newRawElementList;
    m_selectionState = selectionState;

    emit elementsUpdated(m_rawElementList);
}

QList<Ms::EngravingItem*> ElementRepositoryService::findElementsByType(const Ms::ElementType elementType) const
{
    switch (elementType) {
    case Ms::ElementType::CHORD: return findChords();
    case Ms::ElementType::NOTE: return findNotes();
    case Ms::ElementType::NOTEHEAD: return findNoteHeads();
    case Ms::ElementType::STEM: return findStems();
    case Ms::ElementType::HOOK: return findHooks();
    case Ms::ElementType::BEAM: return findBeams();
    case Ms::ElementType::STAFF: return findStaffs();
    case Ms::ElementType::LAYOUT_BREAK: return findSectionBreaks(); //Page breaks and line breaks are of type LAYOUT_BREAK, but they don't appear in the inspector for now.
    case Ms::ElementType::CLEF: return findPairedClefs();
    case Ms::ElementType::TEXT: return findTexts();
    case Ms::ElementType::TREMOLO: return findTremolos();
    case Ms::ElementType::BRACKET: return findBrackets();
    case Ms::ElementType::PEDAL:
    case Ms::ElementType::GLISSANDO:
    case Ms::ElementType::VIBRATO:
    case Ms::ElementType::HAIRPIN:
    case Ms::ElementType::VOLTA:
    case Ms::ElementType::LET_RING:
    case Ms::ElementType::OTTAVA:
    case Ms::ElementType::TEXTLINE:
    case Ms::ElementType::PALM_MUTE: return findLines(elementType);
    default:
        QList<Ms::EngravingItem*> resultList;

        for (Ms::EngravingItem* element : m_exposedElementList) {
            if (element->type() == elementType) {
                resultList << element;
            }
        }

        return resultList;
    }
}

QList<Ms::EngravingItem*> ElementRepositoryService::findElementsByType(const Ms::ElementType elementType,
                                                                       std::function<bool(const Ms::EngravingItem*)> filterFunc) const
{
    QList<Ms::EngravingItem*> resultList;

    QList<Ms::EngravingItem*> unfilteredList = findElementsByType(elementType);

    for (Ms::EngravingItem* element : unfilteredList) {
        if (filterFunc(element)) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::takeAllElements() const
{
    return m_exposedElementList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::exposeRawElements(const QList<Ms::EngravingItem*>& rawElementList) const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : rawElementList) {
        Ms::ElementType elementType = element->type();

        //! NOTE: instrument names can't survive the layout process,
        //! so we have to exclude them from the list to prevent
        //! crashes on invalid pointers in the inspector
        if (elementType == Ms::ElementType::INSTRUMENT_NAME) {
            continue;
        }

        if (elementType == Ms::ElementType::BRACKET) {
            resultList << Ms::toBracket(element)->bracketItem();
            continue;
        }

        if (!resultList.contains(element->elementBase())) {
            resultList << element->elementBase();
        }

        if (elementType == Ms::ElementType::BEAM) {
            const Ms::Beam* beam = Ms::toBeam(element);

            for (Ms::ChordRest* chordRest : beam->elements()) {
                resultList << chordRest;
            }
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findChords() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_exposedElementList) {
        if (element->type() == Ms::ElementType::CHORD) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findNotes() const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : findChords()) {
        const Ms::Chord* chord = Ms::toChord(element);

        if (!chord) {
            continue;
        }

        for (Ms::EngravingItem* note : chord->notes()) {
            resultList << note;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findNoteHeads() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_rawElementList) {
        if (element->isNote()) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findStems() const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : findChords()) {
        const Ms::Chord* chord = Ms::toChord(element);

        if (chord && chord->stem()) {
            resultList << chord->stem();
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findHooks() const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : findChords()) {
        const Ms::Chord* chord = Ms::toChord(element);

        if (chord && chord->hook()) {
            resultList << chord->hook();
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findBeams() const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : findChords()) {
        Ms::EngravingItem* beam = nullptr;

        if (element->isChord()) {
            const Ms::Chord* chord = Ms::toChord(element);

            if (!chord) {
                continue;
            }

            beam = chord->beam();
        } else if (element->isBeam()) {
            beam = const_cast<Ms::EngravingItem*>(element);
        }

        if (!beam || resultList.contains(beam)) {
            continue;
        }

        resultList << beam;
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findLines(Ms::ElementType lineType) const
{
    static const QMap<Ms::ElementType, Ms::ElementType> lineTypeToSegmentType {
        { Ms::ElementType::GLISSANDO, Ms::ElementType::GLISSANDO_SEGMENT },
        { Ms::ElementType::VIBRATO, Ms::ElementType::VIBRATO_SEGMENT },
        { Ms::ElementType::PEDAL, Ms::ElementType::PEDAL_SEGMENT },
        { Ms::ElementType::HAIRPIN, Ms::ElementType::HAIRPIN_SEGMENT },
        { Ms::ElementType::VOLTA, Ms::ElementType::VOLTA_SEGMENT },
        { Ms::ElementType::LET_RING, Ms::ElementType::LET_RING_SEGMENT },
        { Ms::ElementType::PALM_MUTE, Ms::ElementType::PALM_MUTE_SEGMENT },
        { Ms::ElementType::OTTAVA, Ms::ElementType::OTTAVA_SEGMENT },
        { Ms::ElementType::TEXTLINE, Ms::ElementType::TEXTLINE_SEGMENT }
    };

    QList<Ms::EngravingItem*> resultList;

    IF_ASSERT_FAILED(lineTypeToSegmentType.contains(lineType)) {
        return resultList;
    }

    Ms::ElementType segmentType = lineTypeToSegmentType[lineType];

    for (Ms::EngravingItem* element : m_exposedElementList) {
        if (element->type() == segmentType) {
            const Ms::LineSegment* segment = Ms::toLineSegment(element);
            Ms::SLine* line = segment ? segment->line() : nullptr;

            if (line) {
                resultList << line;
            }
        } else if (element->type() == lineType) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findStaffs() const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : m_exposedElementList) {
        if (!element->staff()) {
            continue;
        }

        resultList << element->staff();
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findSectionBreaks() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_exposedElementList) {
        if (element && element->type() == Ms::ElementType::LAYOUT_BREAK) {
            const Ms::LayoutBreak* layoutBreak = Ms::toLayoutBreak(element);
            if (layoutBreak->layoutBreakType() != Ms::LayoutBreakType::SECTION) {
                continue;
            }

            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findPairedClefs() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_exposedElementList) {
        if (element->type() == Ms::ElementType::CLEF) {
            auto clef = Ms::toClef(element);
            IF_ASSERT_FAILED(clef) {
                continue;
            }

            resultList << clef; //could be both main clef and courtesy clef

            auto courtesyPairClef = clef->otherClef(); //seeking for a "pair" clef
            if (courtesyPairClef) {
                resultList << courtesyPairClef;
            }
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findTexts() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_exposedElementList) {
        if (TEXT_ELEMENT_TYPES.contains(element->type())) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findTremolos() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_exposedElementList) {
        if (element->isTremolo()) {
            // the tremolo section currently only has a style setting
            // so only tremolos which can have custom styles make it appear
            if (Ms::toTremolo(element)->customStyleApplicable()) {
                resultList << element;
            }
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findBrackets() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_exposedElementList) {
        if (element->isBracketItem()) {
            resultList << element;
        }
    }

    return resultList;
}
