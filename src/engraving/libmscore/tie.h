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

namespace Ms {
//---------------------------------------------------------
//   @@ TieSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class TieSegment final : public SlurTieSegment
{
    mu::PointF autoAdjustOffset;

    void setAutoAdjust(const mu::PointF& offset);
    void setAutoAdjust(qreal x, qreal y) { setAutoAdjust(mu::PointF(x, y)); }
    mu::PointF getAutoAdjust() const { return autoAdjustOffset; }

protected:
    void changeAnchor(EditData&, Element*) override;

public:
    TieSegment(Score* s)
        : SlurTieSegment(s) { autoAdjustOffset = mu::PointF(); }
    TieSegment(const TieSegment& s)
        : SlurTieSegment(s) { autoAdjustOffset = mu::PointF(); }

    TieSegment* clone() const override { return new TieSegment(*this); }
    ElementType type() const override { return ElementType::TIE_SEGMENT; }
    int subtype() const override { return static_cast<int>(spanner()->type()); }
    void draw(mu::draw::Painter*) const override;

    void layoutSegment(const mu::PointF& p1, const mu::PointF& p2);

    bool isEdited() const;
    void editDrag(EditData&) override;
    bool edit(EditData&) override;

    Tie* tie() const { return (Tie*)spanner(); }

    void computeBezier(mu::PointF so = mu::PointF()) override;
};

//---------------------------------------------------------
//   @@ Tie
//!    a Tie has a Note as startElement/endElement
//---------------------------------------------------------

class Tie final : public SlurTie
{
    static Note* editStartNote;
    static Note* editEndNote;

public:
    Tie(Score* = 0);

    Tie* clone() const override { return new Tie(*this); }
    ElementType type() const override { return ElementType::TIE; }

    void setStartNote(Note* note);
    void setEndNote(Note* note) { setEndElement((Element*)note); }
    Note* startNote() const;
    Note* endNote() const;

    void calculateDirection();
    void write(XmlWriter& xml) const override;
    void slurPos(SlurPos*) override;

    TieSegment* layoutFor(System*);
    TieSegment* layoutBack(System*);

    TieSegment* frontSegment() { return toTieSegment(Spanner::frontSegment()); }
    const TieSegment* frontSegment() const { return toTieSegment(Spanner::frontSegment()); }
    TieSegment* backSegment() { return toTieSegment(Spanner::backSegment()); }
    const TieSegment* backSegment() const { return toTieSegment(Spanner::backSegment()); }
    TieSegment* segmentAt(int n) { return toTieSegment(Spanner::segmentAt(n)); }
    const TieSegment* segmentAt(int n) const { return toTieSegment(Spanner::segmentAt(n)); }

    SlurTieSegment* newSlurTieSegment() override { return new TieSegment(score()); }
};
}     // namespace Ms
#endif
