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

#ifndef __SLUR_H__
#define __SLUR_H__

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

public:
    SlurSegment(System* parent);
    SlurSegment(const SlurSegment& ss);

    SlurSegment* clone() const override { return new SlurSegment(*this); }
    int subtype() const override { return static_cast<int>(spanner()->type()); }
    void draw(mu::draw::Painter*) const override;

    double extraHeight() const { return m_extraHeight; }
    void setExtraHeight(double h) { m_extraHeight = h; }

    bool isEdited() const;
    bool isEndPointsEdited() const;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;

    void editDrag(EditData& ed) override;

    Slur* slur() const { return toSlur(spanner()); }
    void adjustEndpoints();
    void computeBezier(mu::PointF so = mu::PointF()) override;
    Shape getSegmentShape(Segment* seg, ChordRest* startCR, ChordRest* endCR);
    void avoidCollisions(PointF& pp1, PointF& p2, PointF& p3, PointF& p4, mu::draw::Transform& toSystemCoordinates, double& slurAngle);

protected:
    void changeAnchor(EditData&, EngravingItem*) override;
    double m_extraHeight = 0.0;
    PointF m_endPointOff1 = PointF(0.0, 0.0);
    PointF m_endPointOff2 = PointF(0.0, 0.0);
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

    ~Slur() {}

    Slur* clone() const override { return new Slur(*this); }

    void setTrack(track_idx_t val) override;

    int sourceStemArrangement() const { return m_sourceStemArrangement; }
    void setSourceStemArrangement(int v) { m_sourceStemArrangement = v; }

    SlurSegment* frontSegment() { return toSlurSegment(Spanner::frontSegment()); }
    const SlurSegment* frontSegment() const { return toSlurSegment(Spanner::frontSegment()); }
    SlurSegment* backSegment() { return toSlurSegment(Spanner::backSegment()); }
    const SlurSegment* backSegment() const { return toSlurSegment(Spanner::backSegment()); }
    SlurSegment* segmentAt(int n) { return toSlurSegment(Spanner::segmentAt(n)); }
    const SlurSegment* segmentAt(int n) const { return toSlurSegment(Spanner::segmentAt(n)); }

    bool isCrossStaff();
    bool isOverBeams();
    bool stemSideForBeam(bool start);
    bool stemSideStartForBeam() { return stemSideForBeam(true); }
    bool stemSideEndForBeam() { return stemSideForBeam(false); }
    const StemFloated& stemFloated() const { return m_stemFloated; }
    StemFloated& stemFloated() { return m_stemFloated; }

    SlurTieSegment* newSlurTieSegment(System* parent) override { return new SlurSegment(parent); }

    static int calcStemArrangement(EngravingItem* start, EngravingItem* end);
    static bool isDirectionMixture(Chord* c1, Chord* c2);

private:

    friend class Factory;
    Slur(EngravingItem* parent);
    Slur(const Slur&);

    void slurPosChord(SlurPos*);

    int m_sourceStemArrangement = -1;

    StemFloated m_stemFloated; // end point position is attached to stem but floated towards the note
};
} // namespace mu::engraving
#endif
