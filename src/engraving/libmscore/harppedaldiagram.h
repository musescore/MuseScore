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

#ifndef __HARPPEDALDIAGRAM_H__
#define __HARPPEDALDIAGRAM_H__

#include "textbase.h"

namespace mu::engraving {
enum class PedalPosition {
    FLAT,
    NATURAL,
    SHARP
};

// Use for indexes of _pedalState
enum HarpString {
    D, C, B, E, F, G, A
};

class HarpPedalDiagram final : public TextBase
{
    std::vector<PedalPosition> _pedalState;

    bool _isDiagram = true;

public:
    // Constructors - too many?
    HarpPedalDiagram(Segment* parent);
    HarpPedalDiagram(const HarpPedalDiagram& h);
    //HarpPedalDiagram(std::vector<PedalPosition> pedalState);
    //HarpPedalDiagram(PedalPosition d, PedalPosition c, PedalPosition b, PedalPosition e, PedalPosition f, PedalPosition g, PedalPosition a);

    HarpPedalDiagram* clone() const override { return new HarpPedalDiagram(*this); }
    bool isEditable() const override { return false; }

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    void layout() override;

    //TODO
    // xml read/write
    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    //String accessibleInfo() const;
    //String screenReaderInfo() const;

    void setIsDiagram(bool diagram);
    bool isDiagram() { return _isDiagram; }

    std::vector<PedalPosition> getPedalState() { return _pedalState; }
    void setPedalState(std::vector<PedalPosition> state);
    void setPedal(HarpString harpString, PedalPosition pedal);

private:
    //friend class Factory;

    void updateDiagramText();

    const String getStringName(HarpString str);

    HarpPedalDiagram* searchPrevDiagram();
};
} // namespace mu::engraving

#endif // __HARPPEDALDIAGRAM_H__
