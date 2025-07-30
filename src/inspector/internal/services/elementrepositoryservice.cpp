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
#include "elementrepositoryservice.h"

#include "types/texttypes.h"

#include "engraving/dom/barline.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/note.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/playcounttext.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/text.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/volta.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::engraving;

ElementRepositoryService::ElementRepositoryService(QObject* parent)
    : QObject(parent)
{
}

QObject* ElementRepositoryService::getQObject()
{
    return this;
}

bool ElementRepositoryService::needUpdateElementList(const QList<EngravingItem*>& newRawElementList,
                                                     SelState selectionState) const
{
    return m_rawElementList != newRawElementList || m_selectionState != selectionState;
}

void ElementRepositoryService::updateElementList(const QList<EngravingItem*>& newRawElementList,
                                                 SelState selectionState)
{
    if (!needUpdateElementList(newRawElementList, selectionState)) {
        return;
    }

    m_exposedElementList = exposeRawElements(newRawElementList);
    m_rawElementList = newRawElementList;
    m_selectionState = selectionState;

    emit elementsUpdated(m_rawElementList);
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findElementsByType(const mu::engraving::ElementType elementType) const
{
    switch (elementType) {
    case mu::engraving::ElementType::CHORD: return findChords();
    case mu::engraving::ElementType::NOTE: return findNotes();
    case mu::engraving::ElementType::NOTEHEAD: return findNoteHeads();
    case mu::engraving::ElementType::STEM: return findStems();
    case mu::engraving::ElementType::HOOK: return findHooks();
    case mu::engraving::ElementType::BEAM: return findBeams();
    case mu::engraving::ElementType::STAFF: return findStaffs();
    case mu::engraving::ElementType::LAYOUT_BREAK: return findSectionBreaks(); //Page breaks and line breaks are of type LAYOUT_BREAK, but they don't appear in the inspector for now.
    case mu::engraving::ElementType::TEXT: return findTexts();
    case mu::engraving::ElementType::BRACKET: return findBrackets();
    case mu::engraving::ElementType::REST: return findRests();
    case mu::engraving::ElementType::ORNAMENT: return findOrnaments();
    case mu::engraving::ElementType::LYRICS: return findLyrics();
    case mu::engraving::ElementType::PEDAL:
    case mu::engraving::ElementType::GLISSANDO:
    case mu::engraving::ElementType::VIBRATO:
    case mu::engraving::ElementType::HAIRPIN:
    case mu::engraving::ElementType::VOLTA:
    case mu::engraving::ElementType::LET_RING:
    case mu::engraving::ElementType::OTTAVA:
    case mu::engraving::ElementType::TEXTLINE:
    case mu::engraving::ElementType::NOTELINE:
    case mu::engraving::ElementType::SLUR:
    case mu::engraving::ElementType::HAMMER_ON_PULL_OFF:
    case mu::engraving::ElementType::TIE:
    case mu::engraving::ElementType::LAISSEZ_VIB:
    case mu::engraving::ElementType::PARTIAL_TIE:
    case mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE:
    case mu::engraving::ElementType::PALM_MUTE: return findLines(elementType);
    // case mu::engraving::ElementType::PLAY_COUNT_TEXT: return findPlayCountText();
    default:
        QList<mu::engraving::EngravingItem*> resultList;

        for (mu::engraving::EngravingItem* element : m_exposedElementList) {
            if (element->type() == elementType) {
                resultList << element;
            }
        }

        return resultList;
    }
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findElementsByType(const mu::engraving::ElementType elementType,
                                                                                  std::function<bool(const mu::engraving::EngravingItem*)> filterFunc)
const
{
    QList<mu::engraving::EngravingItem*> resultList;

    QList<mu::engraving::EngravingItem*> unfilteredList = findElementsByType(elementType);

    for (mu::engraving::EngravingItem* element : unfilteredList) {
        if (filterFunc(element)) {
            resultList << element;
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::takeAllElements() const
{
    return m_exposedElementList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::exposeRawElements(const QList<mu::engraving::EngravingItem*>& rawElementList)
const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (mu::engraving::EngravingItem* element : rawElementList) {
        mu::engraving::ElementType elementType = element->type();

        //! NOTE: instrument names can't survive the layout process,
        //! so we have to exclude them from the list to prevent
        //! crashes on invalid pointers in the inspector
        if (elementType == mu::engraving::ElementType::INSTRUMENT_NAME) {
            continue;
        }

        if (elementType == mu::engraving::ElementType::BRACKET) {
            resultList << mu::engraving::toBracket(element)->bracketItem();
            continue;
        }

        resultList << element;
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findChords() const
{
    QSet<mu::engraving::EngravingItem*> elements;

    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (element->type() == mu::engraving::ElementType::CHORD) {
            elements << element;
            continue;
        }

        if (element->type() == mu::engraving::ElementType::BEAM) {
            const mu::engraving::Beam* beam = mu::engraving::toBeam(element);

            for (mu::engraving::ChordRest* chordRest : beam->elements()) {
                if (!chordRest->isChord()) {
                    continue;
                }
                elements << chordRest;
            }
            continue;
        }

        mu::engraving::EngravingItem* chord = element->findAncestor(mu::engraving::ElementType::CHORD);
        if (chord) {
            elements << chord;
        }
    }

    return QList<mu::engraving::EngravingItem*>(elements.begin(), elements.end());
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findNotes() const
{
    QList<mu::engraving::EngravingItem*> result;

    for (engraving::EngravingItem* element : m_rawElementList) {
        if (element->isNote()) {
            result << element;
            continue;
        }

        engraving::EngravingItem* elementBase = element->elementBase();

        if (elementBase->isChord()) {
            engraving::Chord* chord = engraving::toChord(elementBase);

            for (mu::engraving::Note* note : chord->notes()) {
                result << note;
            }
        } else if (elementBase->isBeam()) {
            const mu::engraving::Beam* beam = mu::engraving::toBeam(elementBase);

            for (mu::engraving::ChordRest* chordRest : beam->elements()) {
                if (!chordRest->isChord()) {
                    continue;
                }

                for (mu::engraving::Note* note : engraving::toChord(chordRest)->notes()) {
                    result << note;
                }
            }
        }
    }

    return result;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findNoteHeads() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (mu::engraving::EngravingItem* element : m_rawElementList) {
        if (element->isNote()) {
            resultList << element;
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findStems() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (const mu::engraving::EngravingItem* element : findChords()) {
        const mu::engraving::Chord* chord = mu::engraving::toChord(element);

        if (chord && chord->stem()) {
            resultList << chord->stem();
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findHooks() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (const mu::engraving::EngravingItem* element : findChords()) {
        const mu::engraving::Chord* chord = mu::engraving::toChord(element);

        if (chord && chord->hook()) {
            resultList << chord->hook();
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findBeams() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (const mu::engraving::EngravingItem* element : findChords()) {
        mu::engraving::EngravingItem* beam = nullptr;

        if (element->isChord()) {
            const mu::engraving::Chord* chord = mu::engraving::toChord(element);

            if (!chord) {
                continue;
            }

            beam = chord->beam();
        } else if (element->isBeam()) {
            beam = const_cast<mu::engraving::EngravingItem*>(element);
        }

        if (!beam || resultList.contains(beam)) {
            continue;
        }

        resultList << beam;
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findLines(mu::engraving::ElementType lineType) const
{
    static const QMap<mu::engraving::ElementType, mu::engraving::ElementType> lineTypeToSegmentType {
        { mu::engraving::ElementType::GLISSANDO, mu::engraving::ElementType::GLISSANDO_SEGMENT },
        { mu::engraving::ElementType::VIBRATO, mu::engraving::ElementType::VIBRATO_SEGMENT },
        { mu::engraving::ElementType::PEDAL, mu::engraving::ElementType::PEDAL_SEGMENT },
        { mu::engraving::ElementType::HAIRPIN, mu::engraving::ElementType::HAIRPIN_SEGMENT },
        { mu::engraving::ElementType::VOLTA, mu::engraving::ElementType::VOLTA_SEGMENT },
        { mu::engraving::ElementType::LET_RING, mu::engraving::ElementType::LET_RING_SEGMENT },
        { mu::engraving::ElementType::PALM_MUTE, mu::engraving::ElementType::PALM_MUTE_SEGMENT },
        { mu::engraving::ElementType::OTTAVA, mu::engraving::ElementType::OTTAVA_SEGMENT },
        { mu::engraving::ElementType::TEXTLINE, mu::engraving::ElementType::TEXTLINE_SEGMENT },
        { mu::engraving::ElementType::NOTELINE, mu::engraving::ElementType::NOTELINE_SEGMENT },
        { mu::engraving::ElementType::SLUR, mu::engraving::ElementType::SLUR_SEGMENT },
        { mu::engraving::ElementType::HAMMER_ON_PULL_OFF, mu::engraving::ElementType::HAMMER_ON_PULL_OFF_SEGMENT },
        { mu::engraving::ElementType::TIE, mu::engraving::ElementType::TIE_SEGMENT },
        { mu::engraving::ElementType::LAISSEZ_VIB, mu::engraving::ElementType::LAISSEZ_VIB_SEGMENT },
        { mu::engraving::ElementType::PARTIAL_TIE, mu::engraving::ElementType::PARTIAL_TIE_SEGMENT },
        { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE, mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT }
    };

    QList<mu::engraving::EngravingItem*> resultList;

    IF_ASSERT_FAILED(lineTypeToSegmentType.contains(lineType)) {
        return resultList;
    }

    mu::engraving::ElementType segmentType = lineTypeToSegmentType[lineType];

    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (element->type() == segmentType) {
            const mu::engraving::SpannerSegment* segment = mu::engraving::toSpannerSegment(element);
            mu::engraving::Spanner* line = segment ? segment->spanner() : nullptr;

            if (line) {
                resultList << line;
            }
        } else if (element->type() == lineType) {
            resultList << element;
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findStaffs() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (const mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (!element->staff()) {
            continue;
        }

        resultList << element->staff();
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findSectionBreaks() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (element && element->type() == mu::engraving::ElementType::LAYOUT_BREAK) {
            const mu::engraving::LayoutBreak* layoutBreak = mu::engraving::toLayoutBreak(element);
            if (layoutBreak->layoutBreakType() != mu::engraving::LayoutBreakType::SECTION) {
                continue;
            }

            resultList << element;
        }
    }

    return resultList;
}

EngravingItem* ElementRepositoryService::findTextDelegate(EngravingItem* element) const
{
    if (!element->isBarLine()) {
        return element;
    }

    if (PlayCountText* playCount = toBarLine(element)->playCountText()) {
        return playCount;
    }

    return element;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findTexts() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        EngravingItem* el = findTextDelegate(element);
        if (TEXT_ELEMENT_TYPES.contains(el->type())) {
            resultList << el;
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findBrackets() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (element->isBracketItem()) {
            resultList << element;
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findRests() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (element->isRest()) {
            resultList << element;
        } else if (element->isBeam()) {
            const mu::engraving::Beam* beam = mu::engraving::toBeam(element);

            for (mu::engraving::ChordRest* chordRest : beam->elements()) {
                if (!chordRest->isRest()) {
                    continue;
                }
                resultList << chordRest;
            }
        }
    }

    return resultList;
}

QList<mu::engraving::EngravingItem*> ElementRepositoryService::findOrnaments() const
{
    QList<mu::engraving::EngravingItem*> resultList;

    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (element->isOrnament()) {
            resultList << element;
        } else if (element->isTrill()) {
            resultList << (EngravingItem*)(toTrill(element)->ornament());
        } else if (element->isTrillSegment()) {
            resultList << (EngravingItem*)(toTrillSegment(element)->trill()->ornament());
        }
    }

    return resultList;
}

QList<EngravingItem*> ElementRepositoryService::findLyrics() const
{
    QList<mu::engraving::EngravingItem*> resultList;
    for (mu::engraving::EngravingItem* element : m_exposedElementList) {
        if (element->isLyrics()) {
            resultList << element;
        } else if (element->isPartialLyricsLine()) {
            resultList << element;
        } else if (element->isPartialLyricsLineSegment()) {
            resultList << toPartialLyricsLineSegment(element)->lyricsLine();
        }
    }

    return resultList;
}
