/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "harppedaldiagram.h"

#include "translation.h"
#include "types/typesconv.h"

#include "part.h"
#include "score.h"
#include "segment.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

// Position of separator character in diagram
static constexpr size_t SEPARATOR_IDX = 3;

static const String harpStringTypeToString(HarpStringType type)
{
    switch (type) {
    case HarpStringType::A:
        return String(u"A");
    case HarpStringType::B:
        return String(u"B");
    case HarpStringType::C:
        return String(u"C");
    case HarpStringType::D:
        return String(u"D");
    case HarpStringType::E:
        return String(u"E");
    case HarpStringType::F:
        return String(u"F");
    case HarpStringType::G:
        return String(u"G");
    }
    return String();
}

// STYLE
static const ElementStyle harpPedalDiagramStyle {
    { Sid::harpPedalDiagramPlacement, Pid::PLACEMENT },
    { Sid::harpPedalDiagramMinDistance, Pid::MIN_DISTANCE },
};

static const ElementStyle harpPedalTextDiagramStyle {
    { Sid::harpPedalTextDiagramPlacement, Pid::PLACEMENT },
    { Sid::harpPedalTextDiagramMinDistance, Pid::MIN_DISTANCE },
};

// HarpPedalDiagram
void HarpPedalDiagram::setPlayablePitches()
{
    std::set<int> playablePitches;
    static const std::map<HarpStringType, int> string2pitch = {
        { HarpStringType::C, 0 },
        { HarpStringType::D, 2 },
        { HarpStringType::E, 4 },
        { HarpStringType::F, 5 },
        { HarpStringType::G, 7 },
        { HarpStringType::A, 9 },
        { HarpStringType::B, 11 }
    };
    static const std::map<PedalPosition, int> position2delta = {
        { PedalPosition::FLAT, -1 },
        { PedalPosition::NATURAL, 0 },
        { PedalPosition::SHARP, 1 }
    };

    for (size_t i = 0; i < _pedalState.size(); i++) {
        int stringPitch = string2pitch.at(HarpStringType(i));
        int delta = position2delta.at(_pedalState[i]);
        int resPitch = (stringPitch + delta + 12) % 12;
        playablePitches.insert(resPitch);
    }

    _playablePitches = playablePitches;
}

HarpPedalDiagram::HarpPedalDiagram(Segment* parent)
    : TextBase(ElementType::HARP_DIAGRAM, parent, TextStyleType::HARP_PEDAL_DIAGRAM, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&harpPedalDiagramStyle);
    _pedalState
        = std::array<PedalPosition, HARP_STRING_NO> { PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL,
                                                      PedalPosition::NATURAL,
                                                      PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL };
    setPlayablePitches();
}

HarpPedalDiagram::HarpPedalDiagram(const HarpPedalDiagram& h)
    : TextBase(h)
{
    _pedalState = h._pedalState;
    _isDiagram = h._isDiagram;
    setPlayablePitches();
}

void HarpPedalDiagram::setPedalState(std::array<PedalPosition, HARP_STRING_NO> state)
{
    _pedalState = state;
    setPlayablePitches();
}

void HarpPedalDiagram::setIsDiagram(bool diagram)
{
    _isDiagram = diagram;
    const ElementStyle& styleMap = _isDiagram ? harpPedalDiagramStyle : harpPedalTextDiagramStyle;

    if (_isDiagram) {
        setTextStyleType(TextStyleType::HARP_PEDAL_DIAGRAM);
    } else {
        setTextStyleType(TextStyleType::HARP_PEDAL_TEXT_DIAGRAM);
    }

    initElementStyle(&styleMap);
}

void HarpPedalDiagram::setPedal(HarpStringType harpString, PedalPosition pedal)
{
    _pedalState.at(harpString) = pedal;
    setPlayablePitches();
}

String HarpPedalDiagram::createDiagramText()
{
    String diagram;
    if (_isDiagram) {
        for (size_t idx = 0; idx < _pedalState.size(); idx++) {
            if (idx == SEPARATOR_IDX) {
                // insert separator
                diagram.append(u"<sym>harpPedalDivider</sym>");
            }
            switch (_pedalState[idx]) {
            case PedalPosition::FLAT:
                diagram.append(u"<sym>harpPedalRaised</sym>");
                break;
            case PedalPosition::NATURAL:
                diagram.append(u"<sym>harpPedalCentered</sym>");
                break;
            case PedalPosition::SHARP:
                diagram.append(u"<sym>harpPedalLowered</sym>");
                break;
            case PedalPosition::UNSET:
                break;
            }
        }
    } else {
        // find difference between pedal state of previous diagram and this one
        // if no previous, the pedals are "unset" meaning the whole diagram will be displayed

        constexpr std::array<PedalPosition, HARP_STRING_NO> initState
            = { PedalPosition::UNSET, PedalPosition::UNSET, PedalPosition::UNSET, PedalPosition::UNSET, PedalPosition::UNSET,
                PedalPosition::UNSET, PedalPosition::UNSET };
        std::array<PedalPosition, HARP_STRING_NO> prevState;
        HarpPedalDiagram* prevDiagram = nullptr;
        if (part()) {
            prevDiagram = part()->prevHarpDiagram(segment()->tick());
        }

        if (prevDiagram != nullptr) {
            // If states are the same work backwards until we find the first difference then calculate from there
            // This ensures the diagram is identical to the previous one
            prevState = prevDiagram->getPedalState();
            while (prevState == _pedalState) {
                // if states are the same but previous is a diagram write whole config out
                if (prevDiagram->isDiagram()) {
                    prevState = initState;
                    break;
                }
                // Check if there's a diagram before.  If not, start of score and use init state
                prevDiagram = part()->prevHarpDiagram(prevDiagram->segment()->tick());
                if (prevDiagram != nullptr) {
                    prevState = prevDiagram->getPedalState();
                } else {
                    // set to init state and break loop
                    prevState = initState;
                    break;
                }
            }
        } else {
            prevState = initState;
        }

        String topLine, bottomLine;

        for (size_t idx = 0; idx < _pedalState.size(); idx++) {
            if (_pedalState[idx] != prevState[idx]) {
                String strName = harpStringTypeToString(HarpStringType(idx));
                switch (_pedalState[idx]) {
                case PedalPosition::FLAT:
                    strName.append(u"<sym>csymAccidentalFlat</sym> ");
                    break;
                case PedalPosition::NATURAL:
                    strName.append(u"<sym>csymAccidentalNatural</sym> ");
                    break;
                case PedalPosition::SHARP:
                    strName.append(u"<sym>csymAccidentalSharp</sym> ");
                    break;
                case PedalPosition::UNSET:
                    break;
                }
                if (idx < SEPARATOR_IDX) {
                    bottomLine.append(strName);
                } else {
                    topLine.append(strName);
                }
            }
        }

        if (topLine.size() == 0) {
            diagram.append(bottomLine);
        } else {
            diagram.append(topLine + u"\n" + bottomLine);
        }
    }

    return diagram;
}

void HarpPedalDiagram::updateDiagramText()
{
    undoChangeProperty(Pid::TEXT, createDiagramText(), PropertyFlags::STYLED);
}

bool HarpPedalDiagram::isPitchPlayable(int pitch)
{
    return _playablePitches.find(pitch % 12) != _playablePitches.cend();
}

PropertyValue HarpPedalDiagram::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::HARP_IS_DIAGRAM:
        return _isDiagram;
    default:
        return TextBase::getProperty(propertyId);
    }
}

bool HarpPedalDiagram::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::HARP_IS_DIAGRAM:
        setIsDiagram(v.toBool());
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
            return false;
        }
    }
    return true;
}

PropertyValue HarpPedalDiagram::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::HARP_PEDAL_DIAGRAM;
    case Pid::HARP_IS_DIAGRAM:
        return true;
    default:
        return TextBase::propertyDefault(id);
    }
}

String HarpPedalDiagram::accessibleInfo() const
{
    String rez = score() ? score()->getTextStyleUserName(textStyleType()).translated() : TConv::translatedUserName(textStyleType());
    String s;

    for (size_t idx = 0; idx < _pedalState.size(); idx++) {
        s.append(harpStringTypeToString(HarpStringType(idx)));
        switch (_pedalState[idx]) {
        case PedalPosition::FLAT:
            s.append(u"♭ ");
            break;
        case PedalPosition::NATURAL:
            s.append(u"♮ ");
            break;
        case PedalPosition::SHARP:
            s.append(u"♯ ");
            break;
        case PedalPosition::UNSET:
            s.append(u" unset ");
            break;
        }
    }

    // trim trailing space
    s.truncate(s.size() - 1);

    return String(u"%1: %2").arg(rez, s);
}

String HarpPedalDiagram::screenReaderInfo() const
{
    String rez = score() ? score()->getTextStyleUserName(textStyleType()).translated() : TConv::translatedUserName(textStyleType());
    String s;

    for (size_t idx = 0; idx < _pedalState.size(); idx++) {
        s.append(harpStringTypeToString(HarpStringType(idx)) + u" ");
        switch (_pedalState.at(idx)) {
        case PedalPosition::FLAT:
            s.append(mtrc("engraving", TConv::userName(AccidentalVal::FLAT, true)));
            break;
        case PedalPosition::NATURAL:
            s.append(mtrc("engraving", TConv::userName(AccidentalVal::NATURAL, true)));
            break;
        case PedalPosition::SHARP:
            s.append(mtrc("engraving", TConv::userName(AccidentalVal::SHARP, true)));
            break;
        case PedalPosition::UNSET:
            s.append(u" unset ");
            break;
        }
    }

    return String(u"%1: %2").arg(rez, s);
}
