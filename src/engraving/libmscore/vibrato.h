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

#ifndef __VIBRATO_H__
#define __VIBRATO_H__

#include "line.h"

namespace mu::engraving {
class Vibrato;

//---------------------------------------------------------
//   @@ VibratoSegment
//---------------------------------------------------------

class VibratoSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, VibratoSegment)

    SymIdList _symbols;

    void symbolLine(SymId start, SymId fill);
    void symbolLine(SymId start, SymId fill, SymId end);
    virtual Sid getPropertyStyle(Pid) const override;

public:
    VibratoSegment(Vibrato* sp, System* parent);

    VibratoSegment* clone() const override { return new VibratoSegment(*this); }

    Vibrato* vibrato() const { return toVibrato(spanner()); }

    void draw(mu::draw::Painter*) const override;
    void layout() override;

    EngravingItem* propertyDelegate(Pid) override;

    Shape shape() const override;
    SymIdList symbols() const { return _symbols; }
    void setSymbols(const SymIdList& s) { _symbols = s; }
};

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

class Vibrato final : public SLine
{
    OBJECT_ALLOCATOR(engraving, Vibrato)
    DECLARE_CLASSOF(ElementType::VIBRATO)

    Sid getPropertyStyle(Pid) const override;

private:
    VibratoType _vibratoType;
    bool _playArticulation;

public:
    Vibrato(EngravingItem* parent);
    ~Vibrato();

    Vibrato* clone() const override { return new Vibrato(*this); }

    void layout() override;
    LineSegment* createLineSegment(System* parent) override;

    void undoSetVibratoType(VibratoType val);
    void setVibratoType(VibratoType tt) { _vibratoType = tt; }
    VibratoType vibratoType() const { return _vibratoType; }
    void setPlayArticulation(bool val) { _playArticulation = val; }
    bool playArticulation() const { return _playArticulation; }
    String vibratoTypeUserName() const;

    Segment* segment() const { return (Segment*)explicitParent(); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    String accessibleInfo() const override;
};
} // namespace mu::engraving

#endif
