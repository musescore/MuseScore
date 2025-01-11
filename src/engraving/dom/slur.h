/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "slurtie.h"

#include "global/allocator.h"

namespace mu::engraving {
//---------------------------------------------------------
//   @@ SlurSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class SlurSegment final : public SlurTieSegment
{
    OBJECT_ALLOCATOR(engraving, SlurSegment)
    DECLARE_CLASSOF(ElementType::SLUR_SEGMENT)

    M_PROPERTY2(double, extraHeight, setExtraHeight, 0.0)
    M_PROPERTY2(PointF, endPointOff1, setEndPointOff1, PointF(0.0, 0.0))
    M_PROPERTY2(PointF, endPointOff2, setEndPointOff2, PointF(0.0, 0.0))

public:
    SlurSegment(System* parent);
    SlurSegment(const SlurSegment& ss);

    SlurSegment* clone() const override { return new SlurSegment(*this); }
    int subtype() const override { return static_cast<int>(spanner()->type()); }

    bool isEdited() const;
    bool isEndPointsEdited() const;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;

    void editDrag(EditData& ed) override;

    Slur* slur() const { return toSlur(spanner()); }

    double endWidth() const override;
    double midWidth() const override;
    double dottedWidth() const override;

protected:
    void changeAnchor(EditData&, EngravingItem*) override;
};

//---------------------------------------------------------
//   @@ Slur
//---------------------------------------------------------

class Slur final : public SlurTie
{
    OBJECT_ALLOCATOR(engraving, Slur)
    DECLARE_CLASSOF(ElementType::SLUR)

public:

    struct StemFloated
    {
        bool left = false;
        bool right = false;
        void reset()
        {
            left = false;
            right = false;
        }
    };

    /// temporary HACK for correct guitar pro import
    enum ConnectedElement : unsigned char {
        NONE,
        GLISSANDO,
        HAMMER_ON
    };

    ~Slur() {}

    Slur* clone() const override { return new Slur(*this); }

    void setTrack(track_idx_t val) override;

    PropertyValue getProperty(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

    SlurSegment* frontSegment() { return toSlurSegment(Spanner::frontSegment()); }
    const SlurSegment* frontSegment() const { return toSlurSegment(Spanner::frontSegment()); }
    SlurSegment* backSegment() { return toSlurSegment(Spanner::backSegment()); }
    const SlurSegment* backSegment() const { return toSlurSegment(Spanner::backSegment()); }
    SlurSegment* segmentAt(int n) { return toSlurSegment(Spanner::segmentAt(n)); }
    const SlurSegment* segmentAt(int n) const { return toSlurSegment(Spanner::segmentAt(n)); }

    bool isCrossStaff();
    const StemFloated& stemFloated() const { return m_stemFloated; }
    StemFloated& stemFloated() { return m_stemFloated; }

    SlurTieSegment* newSlurTieSegment(System* parent) override { return new SlurSegment(parent); }

    static int calcStemArrangement(EngravingItem* start, EngravingItem* end);

    double scalingFactor() const override;

    void undoSetIncoming(bool incoming);
    void undoSetOutgoing(bool outgoing);
    bool isIncoming() const;
    bool isOutgoing() const;
private:
    M_PROPERTY2(int, sourceStemArrangement, setSourceStemArrangement, -1)
    M_PROPERTY2(ConnectedElement, connectedElement, setConnectedElement, ConnectedElement::NONE)
    M_PROPERTY2(PartialSpannerDirection, partialSpannerDirection, setPartialSpannerDirection, PartialSpannerDirection::NONE)

    friend class Factory;
    Slur(EngravingItem* parent);
    Slur(const Slur&);

    StemFloated m_stemFloated; // end point position is attached to stem but floated towards the note
};
} // namespace mu::engraving
