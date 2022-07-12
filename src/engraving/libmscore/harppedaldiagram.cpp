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
    updateDiagramText();
}

HarpPedalDiagram::HarpPedalDiagram(const HarpPedalDiagram& h)
    : TextBase(h)
{
    _pedalState = h._pedalState;
    _isDiagram = h._isDiagram;

    updateDiagramText();
}

void HarpPedalDiagram::setPedalState(std::vector<PedalPosition> state)
{
    _pedalState = state;
    updateDiagramText();
}

void HarpPedalDiagram::setIsDiagram(bool diagram)
{
    _isDiagram = diagram;

    updateDiagramText();
}

void HarpPedalDiagram::setPedal(HarpStrings harpString, PedalPosition pedal)
{
    _pedalState.at(harpString) = pedal;

    updateDiagramText();
}

void HarpPedalDiagram::updateDiagramText()
{
    String diagram = String("");
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

    setXmlText(diagram);
}

void HarpPedalDiagram::read(XmlReader& xml)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag = xml.name();
        if (tag == "isDiagram") {
            setIsDiagram(xml.readBool());
            //} else if (tag == "pedalState") {
            //read vector somehow
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
    //write vector of strings somehow
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

    updateDiagramText();
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
