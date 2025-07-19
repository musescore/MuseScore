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

#ifndef MU_ENGRAVING_VIBRATO_H
#define MU_ENGRAVING_VIBRATO_H

#include "line.h"

namespace mu::engraving {
class Vibrato;

//---------------------------------------------------------
//   @@ VibratoSegment
//---------------------------------------------------------

class VibratoSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, VibratoSegment)
    DECLARE_CLASSOF(ElementType::VIBRATO_SEGMENT)

public:
    VibratoSegment(Vibrato* sp, System* parent);

    VibratoSegment* clone() const override { return new VibratoSegment(*this); }

    Vibrato* vibrato() const { return toVibrato(spanner()); }

    EngravingItem* propertyDelegate(Pid) override;

    const SymIdList& symbols() const { return m_symbols; }
    void setSymbols(const SymIdList& s) { m_symbols = s; }

    void symbolLine(SymId start, SymId fill);
    void symbolLine(SymId start, SymId fill, SymId end);

private:
    virtual Sid getPropertyStyle(Pid) const override;

    SymIdList m_symbols;
};

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

class Vibrato final : public SLine
{
    OBJECT_ALLOCATOR(engraving, Vibrato)
    DECLARE_CLASSOF(ElementType::VIBRATO)

public:
    Vibrato(EngravingItem* parent);
    ~Vibrato();

    Vibrato* clone() const override { return new Vibrato(*this); }

    LineSegment* createLineSegment(System* parent) override;
    PointF linePos(Grip grip, System** system) const override;

    void undoSetVibratoType(VibratoType val);
    void setVibratoType(VibratoType tt) { m_vibratoType = tt; }
    VibratoType vibratoType() const { return m_vibratoType; }
    String vibratoTypeUserName() const;

    Segment* segment() const { return (Segment*)explicitParent(); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    String accessibleInfo() const override;

    int subtype() const override { return int(m_vibratoType); }
    TranslatableString subtypeUserName() const override;

private:

    Sid getPropertyStyle(Pid) const override;

    VibratoType m_vibratoType = VibratoType::GUITAR_VIBRATO;
};
} // namespace mu::engraving

#endif
