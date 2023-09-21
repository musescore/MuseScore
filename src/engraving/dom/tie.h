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

#ifndef __TIE_H__
#define __TIE_H__

#include "slurtie.h"

namespace mu::engraving {
//---------------------------------------------------------
//   @@ TieSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class TieSegment final : public SlurTieSegment
{
    OBJECT_ALLOCATOR(engraving, TieSegment)
    DECLARE_CLASSOF(ElementType::TIE_SEGMENT)

    double m_midThickness = 0.0;

    std::array<PointF, static_cast<size_t>(Grip::GRIPS)> m_adjustOffsets = { PointF() };

    /*************************
     * DEPRECATED
     * **********************/
    double shoulderHeightMin = 0.4;
    double shoulderHeightMax = 1.3;
    PointF autoAdjustOffset;
    void setAutoAdjust(const PointF& offset);
    void setAutoAdjust(double x, double y) { setAutoAdjust(PointF(x, y)); }
    PointF getAutoAdjust() const { return autoAdjustOffset; }
    /************************/

protected:
    void changeAnchor(EditData&, EngravingItem*) override;

public:
    TieSegment(System* parent);
    TieSegment(const TieSegment& s);

    TieSegment* clone() const override { return new TieSegment(*this); }

    int subtype() const override { return static_cast<int>(spanner()->type()); }

    void adjustY(const PointF& p1, const PointF& p2);
    void adjustX();

    void addAdjustOffset(const PointF& offset, Grip grip) { m_adjustOffsets[static_cast<size_t>(grip)] += offset; }
    void resetAdjustOffset() { m_adjustOffsets.fill(PointF()); }
    PointF adjustOffset(Grip grip) { return m_adjustOffsets[static_cast<size_t>(grip)]; }
    void consolidateAdjustOffsetIntoUserOffset();

    void finalizeSegment();

    bool isEdited() const;
    void editDrag(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;

    Tie* tie() const { return (Tie*)spanner(); }

    void computeBezier(PointF so = PointF()) override;
    void computeMidThickness(double tieLengthInSp);
    void addLineAttachPoints();
    double midThickness() const { return m_midThickness; }
};

//---------------------------------------------------------
//   @@ Tie
//!    a Tie has a Note as startElement/endElement
//---------------------------------------------------------

class Tie final : public SlurTie
{
    OBJECT_ALLOCATOR(engraving, Tie)
    DECLARE_CLASSOF(ElementType::TIE)

    static Note* editStartNote;
    static Note* editEndNote;

    M_PROPERTY2(TiePlacement, tiePlacement, setTiePlacement, TiePlacement::AUTO)

public:
    Tie(EngravingItem* parent = 0);

    Tie* clone() const override { return new Tie(*this); }

    void setStartNote(Note* note);
    void setEndNote(Note* note) { setEndElement((EngravingItem*)note); }
    Note* startNote() const;
    Note* endNote() const;

    bool isInside() const { return m_isInside; }
    void setIsInside(bool val) { m_isInside = val; }
    bool isOuterTieOfChord(Grip startOrEnd) const;
    bool hasTiedSecondInside() const;
    bool isCrossStaff() const;

    void calculateDirection();
    void calculateIsInside();

    PropertyValue getProperty(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

    TieSegment* frontSegment() { return toTieSegment(Spanner::frontSegment()); }
    const TieSegment* frontSegment() const { return toTieSegment(Spanner::frontSegment()); }
    TieSegment* backSegment() { return toTieSegment(Spanner::backSegment()); }
    const TieSegment* backSegment() const { return toTieSegment(Spanner::backSegment()); }
    TieSegment* segmentAt(int n) { return toTieSegment(Spanner::segmentAt(n)); }
    const TieSegment* segmentAt(int n) const { return toTieSegment(Spanner::segmentAt(n)); }

    SlurTieSegment* newSlurTieSegment(System* parent) override { return new TieSegment(parent); }

private:

    bool m_isInside = false;
};
} // namespace mu::engraving
#endif
