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

#ifndef __CHORDLINE_H__
#define __CHORDLINE_H__

#include "engravingitem.h"
#include "draw/types/painterpath.h"

#include "types/types.h"

namespace mu::engraving {
class Factory;

class Chord;

//---------------------------------------------------------
//   @@ ChordLine
///    bezier line attached to top note of a chord
///    implements fall, doit, plop, bend
//---------------------------------------------------------

class ChordLine final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, ChordLine)
protected:

    ChordLineType _chordLineType;
    bool _straight;
    mu::draw::PainterPath path;
    bool modified;
    double _lengthX;
    double _lengthY;
    Note* _note = nullptr;
    static constexpr double _baseLength = 1.0;

    friend class Factory;

    ChordLine(Chord* parent);
    ChordLine(const ChordLine&);

    bool sameVoiceKerningLimited() const override { return true; }

public:

    ChordLine* clone() const override { return new ChordLine(*this); }

    void setChordLineType(ChordLineType);
    ChordLineType chordLineType() const { return _chordLineType; }
    Chord* chord() const { return (Chord*)(explicitParent()); }
    bool isStraight() const { return _straight; }
    void setStraight(bool straight) { _straight =  straight; }
    void setLengthX(double length) { _lengthX = length; }
    void setLengthY(double length) { _lengthY = length; }

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;
    void layout() override;
    void draw(mu::draw::Painter*) const override;

    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    String accessibleInfo() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return _straight ? 1 : static_cast<int>(path.elementCount()); }
    Grip initialEditModeGrip() const override { return Grip(gripsCount() - 1); }
    Grip defaultGrip() const override { return initialEditModeGrip(); }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    bool isToTheLeft() const { return _chordLineType == ChordLineType::PLOP || _chordLineType == ChordLineType::SCOOP; }
    bool isBelow() const { return _chordLineType == ChordLineType::SCOOP || _chordLineType == ChordLineType::FALL; }

    void setNote(Note* note) { _note = note; }
    Note* note() const { return _note; }
};
} // namespace mu::engraving
#endif
