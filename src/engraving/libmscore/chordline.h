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
class Note;

//---------------------------------------------------------
//   @@ ChordLine
///    bezier line attached to top note of a chord
///    implements fall, doit, plop, bend
//---------------------------------------------------------

class ChordLine final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, ChordLine)
    DECLARE_CLASSOF(ElementType::CHORDLINE)

private:

    bool _straight = false;
    bool _wavy = false;

    ChordLineType _chordLineType = ChordLineType::NOTYPE;
    draw::PainterPath m_path;
    bool m_modified = false;
    double _lengthX = 0.0;
    double _lengthY = 0.0;
    Note* _note = nullptr;

    friend class Factory;

    ChordLine(Chord* parent);
    ChordLine(const ChordLine&);

    bool sameVoiceKerningLimited() const override { return true; }
    bool alwaysKernable() const override { return true; }
    KerningType doComputeKerningType(const EngravingItem* nextItem) const override;

public:

    static constexpr double WAVE_ANGEL = 20;
    static const SymIdList WAVE_SYMBOLS;

    ChordLine* clone() const override { return new ChordLine(*this); }

    Chord* chord() const { return (Chord*)(explicitParent()); }

    void setChordLineType(ChordLineType);
    ChordLineType chordLineType() const { return _chordLineType; }
    bool isStraight() const { return _straight; }
    void setStraight(bool straight) { _straight =  straight; }
    bool isWavy() const { return _wavy; }
    void setWavy(bool wavy) { _wavy =  wavy; }
    void setLengthX(double length) { _lengthX = length; }
    double lengthX() const { return _lengthX; }
    void setLengthY(double length) { _lengthY = length; }
    double lengthY() const { return _lengthY; }
    void setPath(const draw::PainterPath& p) { m_path = p; }
    const draw::PainterPath& path() const { return m_path; }
    void setModified(bool m) { m_modified = m; }
    bool modified() const { return m_modified; }

    const TranslatableString& chordLineTypeName() const;

    void draw(mu::draw::Painter*) const override;

    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    String accessibleInfo() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return _straight ? 1 : static_cast<int>(m_path.elementCount()); }
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
