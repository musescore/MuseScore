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

#include "tie.h"

namespace mu::engraving {
class PartialTieSegment : public TieSegment
{
    OBJECT_ALLOCATOR(engraving, PartialTieSegment)
    DECLARE_CLASSOF(ElementType::PARTIAL_TIE_SEGMENT)

public:
    PartialTieSegment(System* parent);
    PartialTieSegment(const PartialTieSegment& s);

    PartialTieSegment* clone() const override { return new PartialTieSegment(*this); }

    PartialTie* partialTie() const { return (PartialTie*)spanner(); }
    int subtype() const override { return static_cast<int>(spanner()->type()); }
private:
    String formatBarsAndBeats() const override;
};

class PartialTie : public Tie
{
    OBJECT_ALLOCATOR(engraving, PartialTie)
    DECLARE_CLASSOF(ElementType::PARTIAL_TIE);

    M_PROPERTY2(PartialSpannerDirection, partialSpannerDirection, setPartialSpannerDirection, PartialSpannerDirection::OUTGOING)

public:
    PartialTie(Note* parent);

    Note* parentNote() const { return parent() ? toNote(parent()) : nullptr; }

    PartialTie* clone() const override { return new PartialTie(*this); }

    inline bool isOutgoing() const { return _partialSpannerDirection == PartialSpannerDirection::OUTGOING; }

    PropertyValue getProperty(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

    SlurTieSegment* newSlurTieSegment(System* parent) override;

    Note* note() const { return isOutgoing() ? startNote() : endNote(); }
    void setStartNote(Note* note) override;
    void setEndNote(Note* note) override;

    TieJumpPointList* tieJumpPoints() override;
    const TieJumpPointList* tieJumpPoints() const override;
    bool allJumpPointsInactive() const override;

    Note* startNote() const override;

    PartialTieSegment* frontSegment() { return toPartialTieSegment(Spanner::frontSegment()); }
    const PartialTieSegment* frontSegment() const { return toPartialTieSegment(Spanner::frontSegment()); }
    PartialTieSegment* backSegment() { return toPartialTieSegment(Spanner::backSegment()); }
    const PartialTieSegment* backSegment() const { return toPartialTieSegment(Spanner::backSegment()); }
    PartialTieSegment* segmentAt(int n) { return toPartialTieSegment(Spanner::segmentAt(n)); }
    const PartialTieSegment* segmentAt(int n) const { return toPartialTieSegment(Spanner::segmentAt(n)); }
};
}
