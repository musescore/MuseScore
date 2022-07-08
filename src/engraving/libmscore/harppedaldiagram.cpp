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

#include "score.h"
#include "segment.h"

#include "rw/xml.h"

using namespace mu;
using namespace mu::engraving;

// Position of separator character in diagram
static const int SEPARATOR_IDX = 3;

static const String harpStringTypeToString(HarpStringType type)
{
    switch (type) {
    case HarpStringType::A:
        return String("A");
    case HarpStringType::B:
        return String("B");
    case HarpStringType::C:
        return String("C");
    case HarpStringType::D:
        return String("D");
    case HarpStringType::E:
        return String("E");
    case HarpStringType::F:
        return String("F");
    case HarpStringType::G:
        return String("G");
    }
    return String("");
}

// STYLE
static const ElementStyle harpPedalDiagramStyle {
    { Sid::harpPedalDiagramPlacement, Pid::PLACEMENT },
    { Sid::harpPedalDiagramMinDistance, Pid::MIN_DISTANCE },
};

// HarpPedalDiagram
HarpPedalDiagram::HarpPedalDiagram(Segment* parent)
    : TextBase(ElementType::HARP_DIAGRAM, parent, TextStyleType::HARP_PEDAL_DIAGRAM, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&harpPedalDiagramStyle);
    _pedalState
        = std::vector<PedalPosition> { PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL,
                                       PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL };
}

HarpPedalDiagram::HarpPedalDiagram(const HarpPedalDiagram& h)
    : TextBase(h)
{
    _pedalState = h._pedalState;
    _isDiagram = h._isDiagram;
}

void HarpPedalDiagram::layout()
{
    TextBase::layout();
}

void HarpPedalDiagram::setPedalState(std::vector<PedalPosition> state)
{
    _pedalState = state;
}

void HarpPedalDiagram::setIsDiagram(bool diagram)
{
    _isDiagram = diagram;
    if (_isDiagram) {
        setTextStyleType(TextStyleType::HARP_PEDAL_DIAGRAM);
    } else {
        setTextStyleType(TextStyleType::HARP_PEDAL_TEXT_DIAGRAM);
    }
}

void HarpPedalDiagram::setPedal(HarpStringType harpString, PedalPosition pedal)
{
    _pedalState.at(harpString) = pedal;
}

void HarpPedalDiagram::updateDiagramText()
{
    String diagram = String("");
    if (_isDiagram) {
        for (int idx = 0; idx < _pedalState.size(); idx++) {
            if (idx == SEPARATOR_IDX) {
                // insert separator
                diagram.append(String("\ue683"));
            }
            switch (_pedalState[idx]) {
            case PedalPosition::FLAT:
                diagram.append(String("\ue680"));
                break;
            case PedalPosition::NATURAL:
                diagram.append(String("\ue681"));
                break;
            case PedalPosition::SHARP:
                diagram.append(String("\ue682"));
                break;
            case PedalPosition::UNSET:
                break;
            }
        }
    } else {
        // find difference between pedal state of previous diagram and this one
        // if no previous, the pedals are "unset" meaning the whole diagram will be displayed

        std::vector<PedalPosition> initState
            = { PedalPosition::UNSET, PedalPosition::UNSET, PedalPosition::UNSET, PedalPosition::UNSET, PedalPosition::UNSET,
                PedalPosition::UNSET, PedalPosition::UNSET };
        std::vector<PedalPosition> prevState;
        HarpPedalDiagram* prevDiagram = searchPrevDiagram();

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
                prevDiagram = prevDiagram->searchPrevDiagram();
                if (prevDiagram != nullptr) {
                    prevState = prevDiagram->getPedalState();
                } else {
                    // set to init state and break loop
                    prevState = initState;
                }
            }
        } else {
            prevState = initState;
        }

        String topLine = String("");
        String bottomLine = String("");

        for (int idx = 0; idx < _pedalState.size(); idx++) {
            if (_pedalState[idx] != prevState[idx]) {
                String strName = harpStringTypeToString(HarpStringType(idx));
                switch (_pedalState[idx]) {
                case PedalPosition::FLAT:
                    strName.append(String("<sym>accidentalFlat</sym> "));
                    break;
                case PedalPosition::NATURAL:
                    strName.append(String("<sym>accidentalNatural</sym> "));
                    break;
                case PedalPosition::SHARP:
                    strName.append(String("<sym>accidentalSharp</sym> "));
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

        diagram.append(topLine + String("\n") + bottomLine);
    }

    setXmlText(diagram);

    layout();
}

// Goes through all previous segments until one with a harp pedal diagram is found

HarpPedalDiagram* HarpPedalDiagram::searchPrevDiagram()
{
    if (segment() != nullptr) {
        Segment* prevSeg = segment()->prev1();

        while (prevSeg != nullptr) {
            for (EngravingItem* e : prevSeg->annotations()) {
                if (e && e->type() == ElementType::HARP_DIAGRAM) {
                    return toHarpPedalDiagram(e);
                }
            }
            prevSeg = prevSeg->prev1();
        }
    }

    return nullptr;
}

void HarpPedalDiagram::read(XmlReader& xml)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag = xml.name();
        if (tag == "isDiagram") {
            setIsDiagram(xml.readBool());
        } else if (tag == "pedalState") {
            while (xml.readNextStartElement()) {
                const AsciiStringView stringTag = xml.name();
                if (stringTag == "string") {
                    HarpStringType str = HarpStringType(xml.intAttribute("name"));
                    PedalPosition pos = PedalPosition(xml.readInt());
                    setPedal(str, pos);
                } else {
                    xml.unknown();
                }
            }
        } else if (!TextBase::readProperties(xml)) {
            xml.unknown();
        }
    }
}

void HarpPedalDiagram::write(XmlWriter& xml) const
{
    if (!xml.context()->canWrite(this)) {
        return;
    }
    xml.startElement(this);
    writeProperty(xml, Pid::HARP_IS_DIAGRAM);

    // Write vector of harp strings.  Order is always D, C, B, E, F, G, A
    xml.startElement("pedalState");
    for (int idx = 0; idx < _pedalState.size(); idx++) {
        xml.tag("string", { { "name", idx } }, static_cast<int>(_pedalState[idx]));
    }
    xml.endElement();

    TextBase::writeProperties(xml);
    xml.endElement();
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
    return TextBase::accessibleInfo();
}
