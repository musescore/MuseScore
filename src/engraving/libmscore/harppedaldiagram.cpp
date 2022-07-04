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

#include "segment.h"

#include "rw/xml.h"

#include "log.h"

namespace mu::engraving {
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
    updateDiagramText();

    TextBase::layout();
}

void HarpPedalDiagram::setPedalState(std::vector<PedalPosition> state)
{
    _pedalState = state;
}

void HarpPedalDiagram::setIsDiagram(bool diagram)
{
    _isDiagram = diagram;
}

void HarpPedalDiagram::setPedal(HarpString harpString, PedalPosition pedal)
{
    _pedalState.at(harpString) = pedal;
}

void HarpPedalDiagram::updateDiagramText()
{
    String diagram = String("");
    if (_isDiagram) {
        for (int idx = 0; idx < _pedalState.size(); idx++) {
            if (idx == 3) {
                // insert separator
                diagram.append(String("<sym>harpPedalDivider</sym>"));
            }
            switch (_pedalState[idx]) {
            case PedalPosition::FLAT:
                diagram.append(String("<sym>harpPedalRaised</sym>"));
                break;
            case PedalPosition::NATURAL:
                diagram.append(String("<sym>harpPedalCentered</sym>"));
                break;
            case PedalPosition::SHARP:
                diagram.append(String("<sym>harpPedalLowered</sym>"));
                break;
            }
        }
    } else {
        // find difference between pedal state of previous diagram and this one
        // if no previous, assume starting from all natural
        std::vector<PedalPosition> prevState;
        HarpPedalDiagram* prevDiagram = searchPrevDiagram();

        if (prevDiagram != nullptr) {
            prevState = prevDiagram->getPedalState();
        } else {
            prevState = { PedalPosition::FLAT, PedalPosition::FLAT, PedalPosition::FLAT, PedalPosition::FLAT,
                          PedalPosition::FLAT, PedalPosition::FLAT, PedalPosition::FLAT };
        }

        for (int idx = 0; idx < _pedalState.size(); idx++) {
            if (_pedalState[idx] != prevState[idx]) {
                String strName = getStringName(static_cast<HarpString>(idx));
                switch (_pedalState[idx]) {
                case PedalPosition::FLAT:
                    diagram.append(strName + String("<sym>accidentalFlat</sym>, "));
                    break;
                case PedalPosition::NATURAL:
                    diagram.append(strName + String("<sym>accidentalNatural</sym>, "));
                    break;
                case PedalPosition::SHARP:
                    diagram.append(strName + String("<sym>accidentalSharp</sym>, "));
                    break;
                }
            }
        }
        // trailing comma
        if (diagram.size() != 0) {
            diagram.truncate(diagram.size() - 2);
        }
    }

    setXmlText(diagram);
}

const String HarpPedalDiagram::getStringName(HarpString str)
{
    switch (str) {
    case HarpString::A:
        return String("A");
    case HarpString::B:
        return String("B");
    case HarpString::C:
        return String("C");
    case HarpString::D:
        return String("D");
    case HarpString::E:
        return String("E");
    case HarpString::F:
        return String("F");
    case HarpString::G:
        return String("G");
    }
}

// Goes through all previous segments until one with a harp pedal diagram is found

HarpPedalDiagram* HarpPedalDiagram::searchPrevDiagram()
{
    EngravingItem* prevElem = this->prevElement();

    while (prevElem != nullptr) {
        if (prevElem->isType(ElementType::HARP_DIAGRAM)) {
            return toHarpPedalDiagram(prevElem);
        }
        prevElem = prevElem->prevElement();
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
                    HarpString str = HarpString(xml.intAttribute("name"));
                    PedalPosition pos = PedalPosition(xml.readInt());
                    setPedal(str, pos);
                    LOGD() << "String " << str << " : " << " Pedal state " << static_cast<int>(pos);
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
        LOGD() << "String " << idx << " : " << " Pedal state " << static_cast<int>(_pedalState[idx]);
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
}   // namespace mu::engraving
