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

#ifndef MU_ENGRAVING_TRILL_H
#define MU_ENGRAVING_TRILL_H

#include "line.h"

namespace mu::engraving {
class Trill;
class Accidental;

//---------------------------------------------------------
//   @@ TrillSegment
//---------------------------------------------------------

class TrillSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, TrillSegment)
    DECLARE_CLASSOF(ElementType::TRILL_SEGMENT)

public:
    TrillSegment(Trill* sp, System* parent);
    TrillSegment(System* parent);

    Trill* trill() const { return (Trill*)spanner(); }

    TrillSegment* clone() const override { return new TrillSegment(*this); }

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all) override;

    EngravingItem* propertyDelegate(Pid) override;

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    void remove(EngravingItem*) override;

    const SymIdList& symbols() const { return m_symbols; }
    void setSymbols(const SymIdList& s) { m_symbols = s; }

    void symbolLine(SymId start, SymId fill);
    void symbolLine(SymId start, SymId fill, SymId end);

private:
    Sid getPropertyStyle(Pid) const override;

    SymIdList m_symbols;
};

//---------------------------------------------------------
//   @@ Trill
//   @P trillType  enum (Trill.DOWNPRALL_LINE, .PRALLPRALL_LINE, .PURE_LINE, .TRILL_LINE, .UPPRALL_LINE)
//---------------------------------------------------------

class Trill final : public SLine
{
    OBJECT_ALLOCATOR(engraving, Trill)
    DECLARE_CLASSOF(ElementType::TRILL)

public:
    Trill(EngravingItem* parent);
    Trill(const Trill& t);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Trill* clone() const override { return new Trill(*this); }
    EngravingItem* linkedClone() override;

    LineSegment* createLineSegment(System* parent) override;
    void remove(EngravingItem*) override;

    void setTrack(track_idx_t n) override;
    void setScore(Score* s) override;
    void computeStartElement() override;
    static PointF trillLinePos(const SLine* line, Grip grip, System** system);
    PointF linePos(Grip grip, System** system) const override;

    void setTrillType(TrillType tt);
    TrillType trillType() const { return m_trillType; }
    int subtype() const override { return int(m_trillType); }
    TranslatableString subtypeUserName() const override;
    void setOrnamentStyle(OrnamentStyle val) { m_ornamentStyle = val; }
    OrnamentStyle ornamentStyle() const { return m_ornamentStyle; }
    String trillTypeUserName() const;
    Accidental* accidental() const { return m_accidental; }
    void setAccidental(Accidental* a) { m_accidental = a; }
    Chord* cueNoteChord() const { return m_cueNoteChord; }
    void setCueNoteChord(Chord* c) { m_cueNoteChord = c; }

    Segment* segment() const { return (Segment*)explicitParent(); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleInfo() const override;

    Ornament* ornament() const { return m_ornament; }
    void setOrnament(Ornament* o) { m_ornament = o; }

protected:
    void doComputeEndElement() override;

private:

    Sid getPropertyStyle(Pid) const override;

    TrillType m_trillType = TrillType::TRILL_LINE;
    Accidental* m_accidental = nullptr;
    Chord* m_cueNoteChord = nullptr;
    OrnamentStyle m_ornamentStyle = OrnamentStyle::DEFAULT;   // for use in ornaments such as trill
    Ornament* m_ornament = nullptr;
};
} // namespace mu::engraving

#endif
