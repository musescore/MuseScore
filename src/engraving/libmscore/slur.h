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

namespace Ms {
//---------------------------------------------------------
//   @@ SlurSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class SlurSegment final : public SlurTieSegment
{
protected:
    qreal _extraHeight = 0.0;
    void changeAnchor(EditData&, Element*) override;

public:
    SlurSegment(Score* s)
        : SlurTieSegment(s) {}
    SlurSegment(const SlurSegment& ss)
        : SlurTieSegment(ss) {}

    SlurSegment* clone() const override { return new SlurSegment(*this); }
    ElementType type() const override { return ElementType::SLUR_SEGMENT; }
    int subtype() const override { return static_cast<int>(spanner()->type()); }
    void draw(mu::draw::Painter*) const override;

    void layoutSegment(const mu::PointF& p1, const mu::PointF& p2);

    bool isEdited() const;
    bool edit(EditData&) override;

    Slur* slur() const { return toSlur(spanner()); }

    void computeBezier(mu::PointF so = mu::PointF()) override;
};

//---------------------------------------------------------
//   @@ Slur
//---------------------------------------------------------

class Slur final : public SlurTie
{
    void slurPosChord(SlurPos*);

public:
    Slur(Score* = 0);
    ~Slur() {}

    Slur* clone() const override { return new Slur(*this); }
    ElementType type() const override { return ElementType::SLUR; }
    void write(XmlWriter& xml) const override;
    void layout() override;
    SpannerSegment* layoutSystem(System*) override;
    void setTrack(int val) override;
    void slurPos(SlurPos*) override;

    SlurSegment* frontSegment() { return toSlurSegment(Spanner::frontSegment()); }
    const SlurSegment* frontSegment() const { return toSlurSegment(Spanner::frontSegment()); }
    SlurSegment* backSegment() { return toSlurSegment(Spanner::backSegment()); }
    const SlurSegment* backSegment() const { return toSlurSegment(Spanner::backSegment()); }
    SlurSegment* segmentAt(int n) { return toSlurSegment(Spanner::segmentAt(n)); }
    const SlurSegment* segmentAt(int n) const { return toSlurSegment(Spanner::segmentAt(n)); }

    SlurTieSegment* newSlurTieSegment() override { return new SlurSegment(score()); }
};
}     // namespace Ms
#endif
