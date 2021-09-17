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
#include "durationtype.h"
#include "stafftype.h"
#include "mscore.h"

#include "log.h"
#include "types/texttypes.h"

using namespace mu::inspector;

ElementRepositoryService::ElementRepositoryService(QObject* parent)
    : QObject(parent)
{
}

QObject* ElementRepositoryService::getQObject()
{
    return this;
}

void ElementRepositoryService::updateElementList(const QList<Ms::EngravingItem*>& newRawElementList)
{
    m_elementList = exposeRawElements(newRawElementList);

    emit elementsUpdated();
}

QList<Ms::EngravingItem*> ElementRepositoryService::findElementsByType(const Ms::ElementType elementType) const
{
    switch (elementType) {
    case Ms::ElementType::CHORD: return findChords();
    case Ms::ElementType::NOTE: return findNotes();
    case Ms::ElementType::STEM: return findStems();
    case Ms::ElementType::HOOK: return findHooks();
    case Ms::ElementType::BEAM: return findBeams();
    case Ms::ElementType::GLISSANDO: return findGlissandos();
    case Ms::ElementType::HAIRPIN: return findHairpins();
    case Ms::ElementType::VOLTA: return findVoltas();
    case Ms::ElementType::STAFF: return findStaffs();
    case Ms::ElementType::LAYOUT_BREAK: return findSectionBreaks(); //Page breaks and line breaks are of type LAYOUT_BREAK, but they don't appear in the inspector for now.
    case Ms::ElementType::PEDAL: return findPedals();
    case Ms::ElementType::CLEF: return findPairedClefs();
    case Ms::ElementType::TEXT: return findTexts();
    case Ms::ElementType::TREMOLO: return findTremolos();
    default:
        QList<Ms::EngravingItem*> resultList;

        for (Ms::EngravingItem* element : m_elementList) {
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
    return m_elementList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::exposeRawElements(const QList<Ms::EngravingItem*>& rawElementList) const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : rawElementList) {
        if (!resultList.contains(element->elementBase())) {
            resultList << element->elementBase();
        }

        if (element->type() == Ms::ElementType::BEAM) {
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

    for (Ms::EngravingItem* element : m_elementList) {
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

QList<Ms::EngravingItem*> ElementRepositoryService::findGlissandos() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_elementList) {
        if (element->type() == Ms::ElementType::GLISSANDO_SEGMENT) {
            const Ms::GlissandoSegment* glissandoSegment = Ms::toGlissandoSegment(element);

            if (!glissandoSegment) {
                continue;
            }

            resultList << glissandoSegment->glissando();
        } else if (element->type() == Ms::ElementType::GLISSANDO) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findVoltas() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_elementList) {
        if (element->type() == Ms::ElementType::VOLTA_SEGMENT) {
            const Ms::VoltaSegment* voltaSegment = Ms::toVoltaSegment(element);

            if (!voltaSegment) {
                continue;
            }

            resultList << voltaSegment->volta();
        } else if (element->type() == Ms::ElementType::VOLTA) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findHairpins() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_elementList) {
        if (element->type() == Ms::ElementType::HAIRPIN_SEGMENT) {
            const Ms::HairpinSegment* hairpinSegment = Ms::toHairpinSegment(element);

            if (!hairpinSegment) {
                continue;
            }

            resultList << hairpinSegment->hairpin();
        } else if (element->type() == Ms::ElementType::HAIRPIN) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findStaffs() const
{
    QList<Ms::EngravingItem*> resultList;

    for (const Ms::EngravingItem* element : m_elementList) {
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

    for (Ms::EngravingItem* element : m_elementList) {
        if (element && element->type() == Ms::ElementType::LAYOUT_BREAK) {
            const Ms::LayoutBreak* layoutBreak = Ms::toLayoutBreak(element);
            if (layoutBreak->layoutBreakType() != Ms::LayoutBreak::Type::SECTION) {
                continue;
            }

            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findPedals() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_elementList) {
        if (element->type() == Ms::ElementType::PEDAL_SEGMENT) {
            const Ms::PedalSegment* pedalSegment = Ms::toPedalSegment(element);

            if (!pedalSegment) {
                continue;
            }

            resultList << pedalSegment->pedal();
        } else if (element->type() == Ms::ElementType::PEDAL) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findPairedClefs() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_elementList) {
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

    for (Ms::EngravingItem* element : m_elementList) {
        if (TEXT_ELEMENT_TYPES.contains(element->type())) {
            resultList << element;
        }
    }

    return resultList;
}

QList<Ms::EngravingItem*> ElementRepositoryService::findTremolos() const
{
    QList<Ms::EngravingItem*> resultList;

    for (Ms::EngravingItem* element : m_elementList) {
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
