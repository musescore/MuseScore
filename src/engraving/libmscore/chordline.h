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

public:

    static constexpr double WAVE_ANGEL = 20;
    static const SymIdList WAVE_SYMBOLS;

    ChordLine* clone() const override { return new ChordLine(*this); }

    Chord* chord() const { return (Chord*)(explicitParent()); }

    void setChordLineType(ChordLineType);
    ChordLineType chordLineType() const { return m_chordLineType; }
    bool isStraight() const { return m_straight; }
    void setStraight(bool straight) { m_straight =  straight; }
    bool isWavy() const { return m_wavy; }
    void setWavy(bool wavy) { m_wavy =  wavy; }
    void setLengthX(double length) { m_lengthX = length; }
    double lengthX() const { return m_lengthX; }
    void setLengthY(double length) { m_lengthY = length; }
    double lengthY() const { return m_lengthY; }
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
    int gripsCount() const override { return m_straight ? 1 : static_cast<int>(m_path.elementCount()); }
    Grip initialEditModeGrip() const override { return Grip(gripsCount() - 1); }
    Grip defaultGrip() const override { return initialEditModeGrip(); }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    bool isToTheLeft() const { return m_chordLineType == ChordLineType::PLOP || m_chordLineType == ChordLineType::SCOOP; }
    bool isBelow() const { return m_chordLineType == ChordLineType::SCOOP || m_chordLineType == ChordLineType::FALL; }

    void setNote(Note* note);
    Note* note() const { return m_note; }

private:

    friend class Factory;

    ChordLine(Chord* parent);
    ChordLine(const ChordLine&);

    bool m_straight = false;
    bool m_wavy = false;

    ChordLineType m_chordLineType = ChordLineType::NOTYPE;
    draw::PainterPath m_path;
    bool m_modified = false;
    double m_lengthX = 0.0;
    double m_lengthY = 0.0;
    Note* m_note = nullptr;
};
} // namespace mu::engraving
#endif
