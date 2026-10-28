/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <array>

#include "engravingitem.h"
#include "draw/types/painterpath.h"

#include "../types/types.h"

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

    ChordLine* clone() const override { return new ChordLine(*this); }

    Chord* chord() const { return (Chord*)(ownershipParent()); }

    void setChordLineType(ChordLineType);
    ChordLineType chordLineType() const { return m_chordLineType; }
    bool isStraight() const { return m_straight; }
    void setStraight(bool straight) { m_straight =  straight; }
    bool isWavy() const { return m_wavy; }
    void setWavy(bool wavy) { m_wavy =  wavy; }

    // The shape is described by up to four points. The default points are in units of the chord line's
    // base length (i.e. spatium scaled by the chord's intrinsic mag), the user's grip offsets are absolute
    enum class Point : unsigned char {
        START,
        BEZIER1,
        BEZIER2,
        END,
        POINTS_COUNT
    };
    using Points = std::array<PointF, static_cast<size_t>(Point::POINTS_COUNT)>;

    Points defaultPoints() const;
    const Points& gripOffsets() const { return m_gripOffsets; }
    const PointF& gripOffset(Point point) const { return m_gripOffsets[size_t(point)]; }
    void setGripOffset(Point point, const PointF& offset) { m_gripOffsets[size_t(point)] = offset; }
    static Pid gripOffsetPid(Point point);

    double baseLength() const;

    const TranslatableString& chordLineTypeName() const;

    void startDragGrip(EditData&) override;
    void dragGrip(EditData&) override;

    String accessibleInfo() const override;

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    void reset() override;

    void spatiumChanged(double oldValue, double newValue) override;
    void localSpatiumChanged(double oldValue, double newValue) override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override;
    Grip initialEditModeGrip() const override { return Grip(gripsCount() - 1); }
    Grip defaultGrip() const override { return initialEditModeGrip(); }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    bool isToTheLeft() const { return m_chordLineType == ChordLineType::PLOP || m_chordLineType == ChordLineType::SCOOP; }
    bool isBelow() const { return m_chordLineType == ChordLineType::SCOOP || m_chordLineType == ChordLineType::FALL; }

    bool playChordLine() const { return m_playChordLine; }
    void setPlayChordLine(bool val) { m_playChordLine = val; }

    void setNote(Note* note);
    Note* note() const { return m_note; }

    SymId waveSym() const;

    struct LayoutData : public EngravingItem::LayoutData {
        muse::draw::PainterPath path;
    };
    DECLARE_LAYOUTDATA_METHODS(ChordLine)

private:

    friend class Factory;

    ChordLine(DummyParentOr<Chord> parent);
    ChordLine(const ChordLine&);

    Point gripPoint(Grip grip) const;

    bool m_straight = false;
    bool m_wavy = false;

    ChordLineType m_chordLineType = ChordLineType::NOTYPE;
    Points m_gripOffsets;
    bool m_playChordLine = true;
    Note* m_note = nullptr;
};
}
