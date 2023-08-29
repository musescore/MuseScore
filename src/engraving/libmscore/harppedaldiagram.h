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

#ifndef __HARPPEDALDIAGRAM_H__
#define __HARPPEDALDIAGRAM_H__

#include "pitchspelling.h"
#include "textbase.h"
#include "pitchspelling.h"

using namespace mu;

namespace mu::engraving {
enum class PedalPosition : char {
    FLAT,
    NATURAL,
    SHARP,

    UNSET                   // Only used in setDiagramText to represent the beginning of a score
};

// Use for indexes of _pedalState
enum HarpStringType : char {
    D, C, B, E, F, G, A
};

// Number of strings per octave on a harp
static constexpr int HARP_STRING_NO = 7;

class HarpPedalDiagram final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, HarpPedalDiagram)
    DECLARE_CLASSOF(ElementType::HARP_DIAGRAM)

public:
    HarpPedalDiagram(Segment* parent);
    HarpPedalDiagram(const HarpPedalDiagram& h);

    HarpPedalDiagram* clone() const override { return new HarpPedalDiagram(*this); }
    bool isEditable() const override { return false; }

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    String accessibleInfo() const override;
    String screenReaderInfo() const override;

    void setIsDiagram(bool diagram);
    bool isDiagram() { return _isDiagram; }

    std::array<PedalPosition, HARP_STRING_NO> getPedalState() const { return _pedalState; }
    void setPedalState(std::array<PedalPosition, HARP_STRING_NO> state);

    void setPedal(HarpStringType harpString, PedalPosition pedal);

    void setPlayableTpcs();

    String createDiagramText();
    void updateDiagramText();

    bool isTpcPlayable(int tpc);

private:

    std::array<PedalPosition, HARP_STRING_NO> _pedalState;

    std::set<int> _playableTpcs;

    bool _isDiagram = true;
};
} // namespace mu::engraving

#endif // __HARPPEDALDIAGRAM_H__
